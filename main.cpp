#include <raylib.h>
#include <vector>
#include <cmath>
#include <numeric>
#include <algorithm>
#include <random>

int windowWidth = 800;
int windowHeight = 800;
int gridSize = 32; // creates 25 x 25 grid with 800 x 800 screen
int tileSize = windowWidth / gridSize;

struct GameCamera {
    float x = 0;          // top-left pixel x of camera viewport
    float y = 0;          // top-left pixel y of camera viewport
    float speed = 800.0f; // pixels per second

    void Update(float deltaTime, int worldPixelWidth, int worldPixelHeight, int screenWidth, int screenHeight) {
        // Move camera based on keys
        if (IsKeyDown(KEY_W)) y -= speed * deltaTime;
        if (IsKeyDown(KEY_S)) y += speed * deltaTime;
        if (IsKeyDown(KEY_A)) x -= speed * deltaTime;
        if (IsKeyDown(KEY_D)) x += speed * deltaTime;

        // Clamp camera inside world bounds
        if (x < 0) x = 0;
        if (y < 0) y = 0;

        float maxX = worldPixelWidth - screenWidth;
        float maxY = worldPixelHeight - screenHeight;

        if (x > maxX) x = maxX > 0 ? maxX : 0;  // prevent negative if world smaller than screen
        if (y > maxY) y = maxY > 0 ? maxY : 0;
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



    void ClearTopRowsToAir(float scale = 0.04f) {
        for (int x = 0; x < width; x++) {
            float noiseVal = perlin.noise(x * scale, (float)generateRandomSeed());
            int clearHeight = mapNoiseToRange(noiseVal, 5, 15); // area that gets cleared
                                                                //
            for (int y = 0; y < clearHeight && y < height; y++) {
                tiles[y * width + x] = TILE_AIR;
            }
        }
    }


    void GenerateTerrain(float scale = 0.1f, float threshold = -0.8f) {
        // first gen 
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                float noiseVal = perlin.noise(x * scale, y * scale); // -1..1
                float dirtNoiseVal = perlin.noise(x * 0.01f, 0.05f * (float)(rand() % 2));
                int dirtHeight = mapNoiseToRange(dirtNoiseVal, 3, dirtDepth);

                if (noiseVal > threshold) {
                    // Stone or dirt depending on y
                    if (y < dirtHeight) {
                        tiles[y * width + x] = TILE_DIRT;
                    } else {
                        tiles[y * width + x] = TILE_STONE;
                    }
                } else {
                    tiles[y * width + x] = TILE_AIR;
                }
            }
        }
        // Worming Terrain
        AddRandWorms(12, 30);
        AddDirtPatches();
        ClearTopRowsToAir();
    }

    
    void Render(const GameCamera& camera, int windowWidth, int windowHeight) {
        for (int y = 0; y < height; ++y) {
            int tilePixelY = y * tileSize;
            bool visibleY = (tilePixelY + tileSize) > camera.y && tilePixelY < camera.y + windowHeight;
            if (!visibleY) continue;

            for (int x = 0; x < width; ++x) {
                int tilePixelX = x * tileSize;
                bool visibleX = (tilePixelX + tileSize) > camera.x && tilePixelX < camera.x + windowWidth;
                if (!visibleX) continue;

                int tile = tiles[y * width + x];
                if (tile == TILE_STONE) {
                    DrawRectangle(tilePixelX - (int)camera.x, tilePixelY - (int)camera.y, tileSize, tileSize, GRAY);
                }
                if (tile == TILE_DIRT) {
                    DrawRectangle(tilePixelX - (int)camera.x, tilePixelY - (int)camera.y, tileSize, tileSize, BROWN);
                }
            }
        }
    }
    int getWidth() const {return width;}
    int getHeight() const {return height;}


private:
    PerlinNoise perlin;
    static constexpr int width = 200;
    static constexpr int height = 200;
    static constexpr int dirtDepth = 80;
    std::vector<int> tiles;
};

int main() {
    InitWindow(windowWidth, windowHeight, "Miner Game");
    SetTargetFPS(60);

    // init
    World world;
    world.GenerateTerrain();

    GameCamera camera;
    const int worldPixelWidth = world.getWidth() * tileSize;
    const int worldPixelHeight = world.getHeight() * tileSize;

    while (!WindowShouldClose()) { // Game Loop
        float deltaTime = GetFrameTime();

        camera.Update(deltaTime, worldPixelWidth, worldPixelHeight, GetScreenWidth(), GetScreenHeight());

        BeginDrawing();

        ClearBackground(BLUE);

        world.Render(camera, GetScreenWidth(), GetScreenHeight());

        EndDrawing();
    }

    CloseWindow();
}
