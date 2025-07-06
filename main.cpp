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
        TILE_STONE = 1
    };

    int& at(int x, int y) {
        return tiles[y * width + x];
    }

    void ClearTopRowsToAir(int rows = 8) {
        for (int y = 0; y < rows; ++y) {
            for (int x = 0; x < width; ++x) {
                tiles[y * width + x] = TILE_AIR;
            }
        }
    }


    void GenerateTerrain(float scale = 0.1f, float threshold = -0.8f) {
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                // Get noise value at scaled coordinates
                float noiseVal = perlin.noise(x * scale, y * scale); // range -1..1
                
                // Threshold noise to decide if tile is filled or empty
                if (noiseVal > threshold) {
                    tiles[y * width + x] = TILE_STONE; // or 1
                } else {
                    tiles[y * width + x] = TILE_AIR; // or 0
                }
            }
        }

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
            }
        }
    }
    int getWidth() const {return width;}
    int getHeight() const {return height;}


private:
    PerlinNoise perlin;
    static constexpr int width = 200;
    static constexpr int height = 200;
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
