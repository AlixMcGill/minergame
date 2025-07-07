#include <iostream>
#include <ostream>
#include <raylib.h>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

int windowWidth = 1280;
int windowHeight = 720;
int gridSize = 32; // creates 25 x 25 grid with 800 x 800 screen
int tileSize = 25;

struct GameCamera {
    float x = 0;
    float y = 0;
    float speed = 1800.0f;

    int drawX = 0; // Integer-aligned position for rendering
    int drawY = 0;

    void Follow(float targetX, float targetY, float deltaTime, int screenWidth, int screenHeight, int worldPixelWidth, int worldPixelHeight) {
        float targetCamX = floor(targetX - screenWidth / 2);
        float targetCamY = floor(targetY - screenHeight / 2);

        float lerpSpeed = 8.0f;

        x += (targetCamX - x) * lerpSpeed * deltaTime;
        y += (targetCamY - y) * lerpSpeed * deltaTime;

        // Clamp camera
        x = std::clamp(x, 0.0f, std::max(0.0f, (float)(worldPixelWidth - screenWidth)));
        y = std::clamp(y, 0.0f, std::max(0.0f, (float)(worldPixelHeight - screenHeight)));

        // Snap for rendering
        drawX = (int)floor(x);
        drawY = (int)floor(y);
    }
};



int mapNoiseToRange(float noiseValue, int min, int max) {
    float normalized = (noiseValue + 1.0f) / 2.0f;
    return static_cast<int>(normalized * (max - min)) + min;
}



unsigned int generateRandomSeed() {
    std::random_device rd;                    // Non-deterministic random device
    std::mt19937 gen(rd());                   // Mersenne Twister engine seeded by rd
    std::uniform_int_distribution<unsigned int> dist(0, UINT32_MAX);
    return dist(gen);
}

class PerlinNoise {
private:
    std::vector<int> p; // Permutation vector

    static double fade(double t) {
        // 6t^5 - 15t^4 + 10t^3 (ease curve)
        return t * t * t * (t * (t * 6 - 15) + 10);
    }

    static double lerp(double a, double b, double t) {
        return a + t * (b - a);
    }

    static double grad(int hash, double x, double y) {
        // Convert low 4 bits of hash code into 12 gradient directions
        int h = hash & 7;  // Use 3 bits
        double u = h < 4 ? x : y;
        double v = h < 4 ? y : x;
        return ((h & 1) ? -u : u) + ((h & 2) ? -2.0 * v : 2.0 * v);
    }

public:
    // Initialize permutation vector with seed
    PerlinNoise(unsigned int seed = 2023) {
        p.resize(256);
        std::iota(p.begin(), p.end(), 0);

        std::default_random_engine engine(seed);
        std::shuffle(p.begin(), p.end(), engine);

        // Duplicate the vector
        p.insert(p.end(), p.begin(), p.end());
    }

    // Generate Perlin noise for coordinate (x, y)
    // Returns value in range [-1, 1]
    double noise(double x, double y) const {
        int X = (int)std::floor(x) & 255;
        int Y = (int)std::floor(y) & 255;

        double xf = x - std::floor(x);
        double yf = y - std::floor(y);

        double u = fade(xf);
        double v = fade(yf);

        int aa = p[p[X] + Y];
        int ab = p[p[X] + Y + 1];
        int ba = p[p[X + 1] + Y];
        int bb = p[p[X + 1] + Y + 1];

        double x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1, yf), u);
        double x2 = lerp(grad(ab, xf, yf - 1), grad(bb, xf - 1, yf - 1), u);

        return lerp(x1, x2, v);
    }
};

class World {
public:
    World() : tiles(width * height, 0), perlin(generateRandomSeed()) {}

    enum TileType {
        TILE_AIR = 0,
        TILE_STONE = 1,
        TILE_DIRT = 2
    };

    int& at(int x, int y) {
        return tiles[y * width + x];
    }

    
    int MapYToRadius(float y, int minRadius, int maxRadius) {
        float t = y / static_cast<float>(height); // top = 0.0, bottom = 1.0
        return static_cast<int>(t * (maxRadius - minRadius)) + minRadius;
    }

    void AddPerlinWorm(int startX, int startY, int length, float noiseScale = 0.1f, int minRadius = 1, int maxRadius = 5) {
        float posX = static_cast<float>(startX);
        float posY = static_cast<float>(startY);

        for (int i = 0; i < length; ++i) {
            float angle = perlin.noise(posX * noiseScale, posY * noiseScale) * 2.0f * 3.14159f;

            posX += cos(angle);
            posY += sin(angle);

            int centerX = static_cast<int>(posX);
            int centerY = static_cast<int>(posY);

            int radius = MapYToRadius(posY, minRadius, maxRadius);

            for (int y = -radius; y <= radius; ++y) {
                for (int x = -radius; x <= radius; ++x) {
                    int nx = centerX + x;
                    int ny = centerY + y;

                    if (nx >= 0 && nx < width && ny >= 0 && ny < height && x*x + y*y <= radius*radius) {
                        tiles[ny * width + nx] = TILE_AIR;
                    }
                }
            }
        }
    }

    void AddRandWorms(int minWorms = 4, int maxWorms = 12) {
        if (maxWorms < minWorms) std::swap(minWorms, maxWorms); // Safety

        int numWorms = minWorms + rand() % (maxWorms - minWorms + 1);

        for (int i = 0; i < numWorms; ++i) {
            int startX = 10 + rand() % (width - 20);
            int startY = 10 + rand() % (height - 20);

            int length = 100 + rand() % 100; // Worm length: 100–199
            float scale = 0.05f + static_cast<float>(rand()) / RAND_MAX * 0.1f; // 0.05–0.15

            int minRadius = 1;
            int maxRadius = 4 + rand() % 3; // 4–6

            AddPerlinWorm(startX, startY, length, scale, minRadius, maxRadius);
        }
    }


    void AddDirtPatches(float noiseScale = 0.08f, float threshold = 0.8f) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                int idx = y * width + x;
                if (tiles[idx] == TILE_STONE) {
                    float noiseVal = perlin.noise(x * noiseScale, y * noiseScale); // range -1..1
                    
                    // Normalize to 0..1
                    float normalized = (noiseVal + 1.0f) / 2.0f;

                    if (normalized > threshold) {
                        tiles[idx] = TILE_DIRT;
                    }
                }
            }
        }
    }

    void ClearTopRowsToAir(float scale = 0.01f) {
        float bandScale = 0.03f; // controls horizontal banding scale

        for (int x = 0; x < width; x++) {
            // Base noise for clear height
            float noiseVal = perlin.noise(x * scale, 0.01f * (float)(rand() % 2));
            int clearHeight = mapNoiseToRange(noiseVal, 20, 50);

            // Second pass horizontal band noise to modify clearHeight
            float xBandNoise = perlin.noise(x * bandScale, 0.0f);
            // Modify clearHeight by ±10% depending on horizontal noise
            int bandOffset = (int)((xBandNoise - 0.5f) * 20); 

            clearHeight += bandOffset;
            if (clearHeight < 0) clearHeight = 0;
            if (clearHeight > height) clearHeight = height;

            for (int y = 0; y < clearHeight && y < height; y++) {
                tiles[y * width + x] = TILE_AIR;
            }
        }
    }

    void InitBasicGen(float scale = 0.06f, float threshold = -1.5f) {
        int octaves = 5;

        for (int y = 0; y < height; ++y) {
            float yFactor = (float)y / (float)height;
            float surfaceFactor = 1.0f + yFactor;           // 1 at top, 0 at bottom
            float airBias = pow(surfaceFactor, 2.0f);       // more effect near surface
            float adjustedThreshold = threshold + airBias * 0.5f;  // increase threshold near surface

            for (int x = 0; x < width; ++x) {
                float noiseVal = 0.0f;
                float frequency = scale;
                float amplitude = 1.0f;
                float maxAmplitude = -1.5f;

                for (int o = 0; o < octaves; ++o) {
                    float nx = x * frequency;
                    float ny = y * frequency;
                    float layerNoise = perlin.noise(nx, ny) * amplitude;
                    noiseVal += layerNoise;
                    maxAmplitude += amplitude;

                    frequency *= 2.0f;
                    amplitude *= 0.5f;
                }

                if (maxAmplitude > 0.0f)
                    noiseVal /= maxAmplitude;

                float dirtNoiseVal = perlin.noise(x * 0.01f, 0.05f * (float)(rand() % 2));
                int dirtHeight = mapNoiseToRange(dirtNoiseVal, 3, dirtDepth);
                if (noiseVal > adjustedThreshold) {
                    if (y < dirtHeight) {
                        tiles[y * width + x] = TILE_DIRT;
                    } else {
                        tiles[y * width + x] = TILE_STONE;
                    }
                } else {
                    tiles[y * width + x] = TILE_AIR;  // more likely near top
                }
                float dirtNoiseVal2 = perlin.noise(x * 0.2f, 0.05f * (float)(rand() % 2));
                int dirtHeight2 = mapNoiseToRange(dirtNoiseVal2, 30, 40);
                if (y < dirtHeight2) {
                    tiles[y * width + x] = TILE_DIRT;
                }
            }
        }
    }


    void GenerateTerrain() {
        InitBasicGen();
        // Worming Terrain
        AddRandWorms(12, 30);
        AddDirtPatches();
        ClearTopRowsToAir();
    }

    bool IsSolidTile(int tileX, int tileY) const {
        if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) return false;

        int tile = tiles[tileY * width + tileX];
        return tile == TILE_STONE || tile == TILE_DIRT;
    }

    
    
    void Render(int camDrawX, int camDrawY, int windowWidth, int windowHeight) {
        int camX = camDrawX;
        int camY = camDrawY;

        for (int y = 0; y < height; ++y) {
            int tilePixelY = y * tileSize;
            bool visibleY = (tilePixelY + tileSize) > camY && tilePixelY < camY + windowHeight;
            if (!visibleY) continue;

            for (int x = 0; x < width; ++x) {
                int tilePixelX = x * tileSize;
                bool visibleX = (tilePixelX + tileSize) > camX && tilePixelX < camX + windowWidth;
                if (!visibleX) continue;

                int tile = tiles[y * width + x];
                if (tile == TILE_STONE) {
                    DrawRectangle((int)(tilePixelX - camX), (int)(tilePixelY - camY), tileSize, tileSize, GRAY);
                    //DrawRectangleLines(tilePixelX - camX, tilePixelY - camY, tileSize, tileSize, RED); // debug border
                }
                else if (tile == TILE_DIRT) {
                    DrawRectangle((int)(tilePixelX - camX), (int)(tilePixelY - camY), tileSize, tileSize, BROWN);
                    //DrawRectangleLines(tilePixelX - camX, tilePixelY - camY, tileSize, tileSize, RED); // debug border
                }
            }
        }
    }


    int getWidth() const {return width;}
    int getHeight() const {return height;}


private:
    PerlinNoise perlin;
    static constexpr int width = 800;
    static constexpr int height = 500;
    static constexpr int dirtDepth = 80;
    std::vector<int> tiles;
};


class Player {
public:
    float x = 0;
    float y = 0;
    float width = 24;
    float height = 24;

    float speed = 300.0f;
    float vx = 0;
    float vy = 0;

    float gravity = 1200.0f;
    float maxFallSpeed = 1200.0f;

    bool isOnGround = false;

    void Update(float deltaTime, const World& world) {
        // Horizontal input
        vx = 0;
        if (IsKeyDown(KEY_A)) vx = -speed;
        if (IsKeyDown(KEY_D)) vx = speed;

        // Jump
        if ((isOnGround || IsOnGroundWithTolerance(world)) && IsKeyPressed(KEY_SPACE)) {
            vy = -600.0f;
            isOnGround = false;
        }

        // Apply gravity
        vy += gravity * deltaTime;
        if (vy > maxFallSpeed) vy = maxFallSpeed;

        // Horizontal movement and collision
        MoveX(vx * deltaTime, world);

        // Vertical movement and collision
        MoveY(vy * deltaTime, world);
    }

    
   
    void MoveX(float dx, const World& world) {
        float sign = dx > 0 ? 1.0f : -1.0f;
        float step = tileSize / 4.0f; // quarter tile steps
        float moved = 0.0f;

        while (std::abs(moved) < std::abs(dx)) {
            float move = std::min(step, std::abs(dx - moved)) * sign;
            x += move;
            if (IsColliding(world)) {
                x -= move;
                vx = 0;
                break;
            }
            moved += move;
        }
    }
        
    void MoveY(float dy, const World& world) {
        float sign = dy > 0 ? 1.0f : -1.0f;
        float step = tileSize / 4.0f;
        float moved = 0.0f;

        while (std::abs(moved) < std::abs(dy)) {
            float move = std::min(step, std::abs(dy - moved)) * sign;
            y += move;
            if (IsColliding(world)) {
                y -= move;
                vy = 0;

                if (dy > 0)
                    isOnGround = true;
                return;
            }
            moved += move;
        }

        isOnGround = false;
    }



    bool IsOnGroundWithTolerance(const World& world, float tolerance = 10.0f) const {
        // Check a small area just below the player's feet
        return IsCollidingAt(x, y + height + tolerance, width, 0.1f, world);
    }

    bool IsCollidingAt(float px, float py, float w, float h, const World& world) const {
        int tileX0 = (int)floor(px / tileSize);
        int tileY0 = (int)floor(py / tileSize);
        int tileX1 = (int)floor((px + w - 0.01f) / tileSize);
        int tileY1 = (int)floor((py + h - 0.01f) / tileSize);

        for (int ty = tileY0; ty <= tileY1; ++ty) {
            for (int tx = tileX0; tx <= tileX1; ++tx) {
                if (world.IsSolidTile(tx, ty)) return true;
            }
        }
        return false;
    }


    bool IsColliding(const World& world) const {
        // Convert player bounding box to tile range
        int tileX0 = (int)floor(x / tileSize);
        int tileY0 = (int)floor(y / tileSize);
        int tileX1 = (int)floor((x + width) / tileSize);
        int tileY1 = (int)floor((y + height) / tileSize);

        for (int ty = tileY0; ty <= tileY1; ++ty) {
            for (int tx = tileX0; tx <= tileX1; ++tx) {
                if (world.IsSolidTile(tx, ty)) {
                    //std::cout << "Colliding with tile at ("<< tx <<", "<< ty <<")" << " Size : " << tileSize << std::endl;
                    return true;
                }
            }
        }
        return false;
    }

    bool IsCollidingAt(float px, float py, float w, float h, const World& world) {
        int tileX0 = (int)floor(px / tileSize);
        int tileY0 = (int)floor(py / tileSize);
        int tileX1 = (int)floor((px + w - 0.01f) / tileSize);
        int tileY1 = (int)floor((py + h - 0.01f) / tileSize);

        for (int ty = tileY0; ty <= tileY1; ++ty) {
            for (int tx = tileX0; tx <= tileX1; ++tx) {
                if (world.IsSolidTile(tx, ty)) return true;
            }
        }
        return false;
    }



    void Draw(int camDrawX, int camDrawY) const {
        DrawRectangle((int)(x - camDrawX), (int)(y - camDrawY), (int)width, (int)height, RED);
    }

    void DebugDrawBounds(const GameCamera& cam) const {
        DrawRectangleLines(
            (int)(x - cam.x),
            (int)(y - cam.y),
            (int)width,
            (int)height,
            GREEN
        );
    }

};

void RenderDebug(const GameCamera& camera, const Player& player, const World& world) {
    // Draw tiles around the player
    int startX = (int)floor(player.x / tileSize) - 2;
    int endX = startX + 5;
    int startY = (int)floor(player.y / tileSize) - 2;
    int endY = startY + 5;

    for (int y = startY; y <= endY; y++) {
        for (int x = startX; x <= endX; x++) {
            if (world.IsSolidTile(x, y)) {
                DrawRectangle(
                    x * tileSize - (int)camera.x,
                    y * tileSize - (int)camera.y,
                    tileSize,
                    tileSize,
                    Fade(GRAY, 0.3f)
                );
            }
        }
    }

    // Draw player collision bounds
    player.DebugDrawBounds(camera);
}



class BlockEditor {
public:
    BlockEditor(World& world, const GameCamera& camera)
        : world(world), camera(camera) {}

    void Update() {
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            int tileX, tileY;
            if (GetHoveredTile(tileX, tileY)) {
                if (tileX >= 0 && tileX < world.getWidth() &&
                    tileY >= 0 && tileY < world.getHeight()) {
                    world.at(tileX, tileY) = World::TILE_AIR;
                }
            }
        }
    }

    void DrawHighlight() const {
        int tileX, tileY;
        if (GetHoveredTile(tileX, tileY)) {
            if (world.IsSolidTile(tileX, tileY)) {
                DrawRectangle(
                    tileX * tileSize - camera.drawX,
                    tileY * tileSize - camera.drawY,
                    tileSize,
                    tileSize,
                    Fade(YELLOW, 0.3f)
                );

                DrawRectangleLines(
                    tileX * tileSize - camera.drawX,
                    tileY * tileSize - camera.drawY,
                    tileSize,
                    tileSize,
                    YELLOW
                );
            }
        }
    }

private:
    World& world;
    const GameCamera& camera;

    bool GetHoveredTile(int& outTileX, int& outTileY) const {
        Vector2 mouse = GetMousePosition();

        // Convert from screen space to world space
        float worldX = mouse.x + camera.drawX;
        float worldY = mouse.y + camera.drawY;

        outTileX = (int)(worldX / tileSize);
        outTileY = (int)(worldY / tileSize);

        return true;
    }
};




int main() {
    InitWindow(windowWidth, windowHeight, "Miner Game");
    SetTargetFPS(240);

    // Init world
    World world;
    world.GenerateTerrain();

    // Init player
    Player player;
    player.x = 200; // Starting position (adjust as needed)
    player.y = 200;

    // Init camera
    GameCamera camera;
    const int worldPixelWidth = world.getWidth() * tileSize;
    const int worldPixelHeight = world.getHeight() * tileSize;

    // Init Block Editor
    BlockEditor editor(world, camera);

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        editor.Update();
        // Update player
        player.Update(deltaTime, world);

        // Camera follows player
        camera.Follow(floorf(player.x), floorf(player.y), deltaTime, GetScreenWidth(), GetScreenHeight(), worldPixelWidth, worldPixelHeight);
        int camDrawX = (int)floor(camera.x);
        int camDrawY = (int)floor(camera.y);
        //std::cout << "camDrawX: " << camDrawX << " camDrawY: " << camDrawY << std::endl;

        
        // Draw
        BeginDrawing();
        ClearBackground(BLUE);

        // Set camera offset
        BeginScissorMode(0, 0, GetScreenWidth(), GetScreenHeight());

        world.Render(camDrawX, camDrawY, GetScreenWidth(), GetScreenHeight());
        player.Draw(camDrawX, camDrawY); // optional: adjust for camera.x, camera.y
        editor.DrawHighlight();
        //RenderDebug(camera, player, world);
        //
        DrawFPS(10, 10);

        EndScissorMode();
        EndDrawing();
    }

    CloseWindow();
}

