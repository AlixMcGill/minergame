#pragma once

#include "../player/player.hpp"
#include "../world/world.hpp"
#include "../util/globals.hpp"
#include "../player/blockEditor.hpp"
#include "../player/camera.hpp"

class Game {
public:
    Game();
    void Update(float deltaTime);
    void Draw();

private:
    World world;
    Player player;
    GameCamera camera;
    BlockEditor editor;

    int worldPixelWidth;
    int worldPixelHeight;
};

