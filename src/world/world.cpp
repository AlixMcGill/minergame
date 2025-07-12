#pragma once
#include "world.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <raylib.h>
#include "../util/perlin.hpp"
#include "../util/utils.hpp"
#include "../util/globals.hpp"
#include "textureManager.hpp"

// You need to define this in one .cpp file
//int tileSize = 16;

World::World() :  perlin(generateRandomSeed()), tiles(width * height, 0) {
}

World::~World() {
}

int& World::at(int x, int y) {
    return tiles[y * width + x];
}

int World::MapYToRadius(float y, int minRadius, int maxRadius) {
    float t = y / static_cast<float>(height);
    return static_cast<int>(t * (maxRadius - minRadius)) + minRadius;
}

bool World::IsSolidTile(int tileX, int tileY) const {
    if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) return false;
    int tile = tiles[tileY * width + tileX];
    return tile == TILE_STONE || tile == TILE_DIRT || tile == TILE_DIRT_GRASS;
}

bool World::IsTile(int tileX, int tileY) const {
    if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) return false;
    int tile = tiles[tileY * width + tileX];
    return tile == TILE_STONE || tile == TILE_DIRT || tile == TILE_DIRT_GRASS || tile == TILE_TREE_TRUNK ||
        tile == TILE_TREE_LEAVES;
}

void World::AddPerlinWorm(int startX, int startY, int length, float noiseScale, int minRadius, int maxRadius) {
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

                if (nx >= 0 && nx < width && ny >= 0 && ny < height && x * x + y * y <= radius * radius) {
                    tiles[ny * width + nx] = TILE_AIR;
                }
            }
        }
    }
}

void World::AddRandWorms(int minWorms, int maxWorms) {
    if (maxWorms < minWorms) std::swap(minWorms, maxWorms);

    int numWorms = minWorms + rand() % (maxWorms - minWorms + 1);

    for (int i = 0; i < numWorms; ++i) {
        int startX = 10 + rand() % (width - 20);
        int startY = 10 + rand() % (height - 20);

        int length = 100 + rand() % 100;
        float scale = 0.05f + static_cast<float>(rand()) / RAND_MAX * 0.1f;

        int minRadius = 1;
        int maxRadius = 4 + rand() % 3;

        AddPerlinWorm(startX, startY, length, scale, minRadius, maxRadius);
    }
}

void World::AddDirtPatches(float noiseScale, float threshold) {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            int idx = y * width + x;
            if (tiles[idx] == TILE_STONE) {
                float noiseVal = perlin.noise(x * noiseScale, y * noiseScale);
                float normalized = (noiseVal + 1.0f) / 2.0f;
                if (normalized > threshold) {
                    tiles[idx] = TILE_DIRT;
                }
            }
        }
    }
}

void World::ClearTopRowsToAir(float scale) {
    float bandScale = 0.01f;

    for (int x = 0; x < width; x++) {
        float noiseVal = perlin.noise(x * scale, 0.01f * (float)(rand() % 2));
        int clearHeight = mapNoiseToRange(noiseVal, 80, 180);

        float xBandNoise = perlin.noise(x * bandScale, 0.0f);
        int bandOffset = (int)((xBandNoise - 0.5f) * 20);

        clearHeight += bandOffset;
        clearHeight = std::clamp(clearHeight, 0, height);

        for (int y = 0; y < clearHeight; y++) {
            tiles[y * width + x] = TILE_AIR;
        }
    }
}

void World::InitBasicGen(float scale, float threshold) {
    int octaves = 5;

    for (int y = 0; y < height; ++y) {
        float yFactor = (float)y / (float)height;
        float surfaceFactor = 1.0f + yFactor;
        float airBias = pow(surfaceFactor, 2.0f);
        float adjustedThreshold = threshold + airBias * 0.5f;

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
            int dirtHeight = mapNoiseToRange(dirtNoiseVal, 70, dirtDepth);

            if (noiseVal > adjustedThreshold) {
                if (y < dirtHeight) {
                    tiles[y * width + x] = TILE_DIRT;
                } else {
                    tiles[y * width + x] = TILE_STONE;
                }
            } else {
                tiles[y * width + x] = TILE_AIR;
            }

            float dirtNoiseVal2 = perlin.noise(x * 0.01f, 0.0001f * (float)(rand() % 2));
            int dirtHeight2 = mapNoiseToRange(dirtNoiseVal2, 90, 120);
            if (y < dirtHeight2) {
                tiles[y * width + x] = TILE_DIRT;
            }
        }
    }
}

void World::AddGrass() {
    for (int y = 5; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (at(x,y) == TILE_DIRT) {
                bool enoughAirAbove = true;
                int numOfAirAbove = 5;
                for (int i = 1; i <= numOfAirAbove; i++) {
                    if (at(x, y - i) != TILE_AIR) {
                        enoughAirAbove = false;
                        break;
                    }
                }
                    if (enoughAirAbove) {
                        at(x,y) = TILE_DIRT_GRASS;
                    }
            }
        }
    }
}

void World::DrawTileLine(int x0, int y0, int x1, int y1, int tileType) {
    int dx = abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        at(x0, y0) = tileType;

        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

bool World::CanPlaceFractalTree(int x, int y, float angle, float length, int depth) {
    if (depth <= 0 || length < 1.0f) return true;

    int x2 = (int)(x + cosf(angle) * length);
    int y2 = (int)(y + sinf(angle) * length);

    // Bresenham-style check from (x, y) to (x2, y2)
    int dx = abs(x2 - x), sx = x < x2 ? 1 : -1;
    int dy = -abs(y2 - y), sy = y < y2 ? 1 : -1;
    int err = dx + dy;

    int cx = x, cy = y;
    while (true) {
        if (IsSolidTile(cx, cy)) return false;

        if (cx == x2 && cy == y2) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; cx += sx; }
        if (e2 <= dx) { err += dx; cy += sy; }
    }

    // Recurse to check branches
    float newLength = length * 0.7f;
    float angleOffset = 0.5f;

    return CanPlaceFractalTree(x2, y2, angle - angleOffset, newLength, depth - 1) &&
           CanPlaceFractalTree(x2, y2, angle + angleOffset, newLength, depth - 1);
}

void World::PlaceLeafCluster(int x, int y) {
    // Offsets for a 3x3 block around (x,y)
    static const int offsets[9][2] = {
        {-1, -1}, {0, -1}, {1, -1},
        {-1,  0}, {0,  0}, {1,  0},
        {-1,  1}, {0,  1}, {1,  1}
    };

    for (auto& offset : offsets) {
        int nx = x + offset[0];
        int ny = y + offset[1];

        // Bounds check
        if (nx < 0 || nx >= width || ny < 0 || ny >= height) continue;

        // Only place leaf if air
        if (at(nx, ny) == TILE_AIR) {
            at(nx, ny) = TILE_TREE_LEAVES;
        }
    }
}


void World::AddFractalTree(int x, int y, float angle, float length, int depth) {
    if (depth <= 0 || length < 1.0f) {
        PlaceLeafCluster(x, y);
        return;
    }

    float x2 = x + cosf(angle) * length;
    float y2 = y + sinf(angle) * length;

    DrawTileLine(x, y, x2, y2, TILE_TREE_TRUNK);

    float newLength = length * 0.7f;
    float angleOffset = 0.5f;

    AddFractalTree((int)x2, (int)y2, angle - angleOffset, newLength, depth - 1);
    AddFractalTree((int)x2, (int)y2, angle + angleOffset, newLength, depth - 1);
}

void World::AddTrees() {
    for (int y = 15; y < height; ++y) { // so we can safely look 15 tiles up
        for (int x = 0; x < width; ++x) {
            if (at(x, y) == TILE_DIRT_GRASS) {
                // Check 15 air tiles above
                bool allAboveAreAir = true;
                for (int i = 1; i <= 15; ++i) {
                    if (at(x, y - i) != TILE_AIR) {
                        allAboveAreAir = false;
                        break;
                    }
                }

                // 15% chance
                if (allAboveAreAir && (rand() % 100 < 15)) {
                    float baseAngle = -PI / 2.0f; // straight up
                    float angleVariance = ((rand() % 100) / 100.0f - 0.5f) * 0.3f; // random between -0.15 and +0.15
                    float randomAngle = baseAngle + angleVariance;
                    if (CanPlaceFractalTree(x, y - 1, randomAngle, 4.0f, 3))
                    AddFractalTree(x, y - 1, randomAngle, 4.0f, 3); // spawn tree going up
                }
            }
        }
    }
}

void World::GenerateTerrain() {
    std::cout << "Initializing Generation..." << std::endl;
    InitBasicGen();

    std::cout << "Adding random long caves." << std::endl;
    AddRandWorms(12, 30);

    std::cout << "Adding dirt patches." << std::endl;
    AddDirtPatches();

    std::cout << "Creating the surface." << std::endl;
    ClearTopRowsToAir(0.00000000001);

    std::cout << "Adding grass." << std::endl;
    AddGrass();

    std::cout << "Adding trees." << std::endl;
    AddTrees();
}


void World::Render(int camDrawX, int camDrawY, int windowWidth, int windowHeight, TextureManager& textureManager) {
    int camX = camDrawX;
    int camY = camDrawY;

    for (int y = 0; y < height; ++y) {
        int tilePixelY = y * tileSize;
        if ((tilePixelY + tileSize) <= camY || tilePixelY >= camY + windowHeight) continue;

        for (int x = 0; x < width; ++x) {
            int tilePixelX = x * tileSize;
            if ((tilePixelX + tileSize) <= camX || tilePixelX >= camX + windowWidth) continue;

            int tile = tiles[y * width + x];
            textureManager.WorldTextureManager(tile, camX, camY, tilePixelX, tilePixelY);
        }
    }
}

int World::getWidth() const { return width; }
int World::getHeight() const { return height; }

int World::GetTileAtWorldPixel(float worldX, float worldY) const {
    int tileX = static_cast<int>(worldX) / tileSize;
    int tileY = static_cast<int>(worldY) / tileSize;

    if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) {
        return -1; // or TILE_AIR if you want a default
    }

    return tiles[tileY * width + tileX];
}

