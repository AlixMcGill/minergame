#pragma once
#include "world.hpp"
#include <cmath>
#include <algorithm>
#include <cstdlib>
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
    float bandScale = 0.03f;

    for (int x = 0; x < width; x++) {
        float noiseVal = perlin.noise(x * scale, 0.01f * (float)(rand() % 2));
        int clearHeight = mapNoiseToRange(noiseVal, 20, 50);

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
            int dirtHeight = mapNoiseToRange(dirtNoiseVal, 3, dirtDepth);

            if (noiseVal > adjustedThreshold) {
                if (y < dirtHeight) {
                    tiles[y * width + x] = TILE_DIRT;
                } else {
                    tiles[y * width + x] = TILE_STONE;
                }
            } else {
                tiles[y * width + x] = TILE_AIR;
            }

            float dirtNoiseVal2 = perlin.noise(x * 0.2f, 0.05f * (float)(rand() % 2));
            int dirtHeight2 = mapNoiseToRange(dirtNoiseVal2, 30, 40);
            if (y < dirtHeight2) {
                tiles[y * width + x] = TILE_DIRT;
            }
        }
    }
}

void World::GenerateTerrain() {
    InitBasicGen();
    AddRandWorms(12, 30);
    AddDirtPatches();
    ClearTopRowsToAir();
}

bool World::IsSolidTile(int tileX, int tileY) const {
    if (tileX < 0 || tileY < 0 || tileX >= width || tileY >= height) return false;
    int tile = tiles[tileY * width + tileX];
    return tile == TILE_STONE || tile == TILE_DIRT;
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

