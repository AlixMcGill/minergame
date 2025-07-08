#pragma once

#include "../world/world.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "../util/utils.hpp"
#include "raylib.h"

class BlockEditor {
public:
    BlockEditor(World& world, const GameCamera& camera);

    void Update(Player& player);
    void DrawHighlight(Player& player) const;

private:
    World& world;
    const GameCamera& camera;
    int blockReach = 5;

    bool GetHoveredTile(int& outTileX, int& outTileY) const;
};

