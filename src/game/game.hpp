#pragma once

#include "../player/player.hpp"
#include "../world/world.hpp"
#include "../world/textureManager.hpp"
#include "../util/globals.hpp"
#include "../player/blockEditor.hpp"
#include "../player/camera.hpp"
#include "itemManager.hpp"

class Game {
public:
    Game();
    void Update(float deltaTime);
    void Draw();
    void Destroy();

private:
    World world;
    Player player;
    GameCamera camera;
    BlockEditor editor;
    ItemManager itemManager;
    TextureManager textrueManager;

    int worldPixelWidth;
    int worldPixelHeight;
};

