#pragma once

#include <vector>
#include <raylib.h>
#include "../util/perlin.hpp"

// You may want to extern tileSize if used outside World
extern int tileSize;

class World {
public:
    enum TileType {
        TILE_AIR = 0,
        TILE_STONE = 1,
        TILE_DIRT = 2
    };

    World();

    int& at(int x, int y);

    void GenerateTerrain();
    void InitBasicGen(float scale = 0.06f, float threshold = -1.5f);
    void ClearTopRowsToAir(float scale = 0.01f);
    void AddDirtPatches(float noiseScale = 0.08f, float threshold = 0.8f);
    void AddRandWorms(int minWorms = 4, int maxWorms = 12);
    void AddPerlinWorm(int startX, int startY, int length, float noiseScale = 0.1f, int minRadius = 1, int maxRadius = 5);

    bool IsSolidTile(int tileX, int tileY) const;

    void Render(int camDrawX, int camDrawY, int windowWidth, int windowHeight);

    int getWidth() const;
    int getHeight() const;

private:
    int MapYToRadius(float y, int minRadius, int maxRadius);
    PerlinNoise perlin;

    static constexpr int width = 800;
    static constexpr int height = 500;
    static constexpr int dirtDepth = 80;
    std::vector<int> tiles;
};

