#include "game.hpp"
#include <iostream>

Game::Game()
    : editor(world, camera) // initialize editor
{
    std::cout << "Game Constructor Started...." << std::endl;

    textrueManager.Load();
    std::cout << "Texture Manager Loaded." << std::endl;

    world.GenerateTerrain();
    std::cout << "Terrain Generated." << std::endl;

    player.Init(world);
    std::cout << "Initialized The Player." << std::endl;

    worldPixelWidth = world.getWidth() * tileSize;
    worldPixelHeight = world.getHeight() * tileSize;
 
    std::cout << "Game Constructor Completed." << std::endl;
}

void Game::Update(float deltaTime) {
    player.Update(deltaTime, world);
    editor.Update(player, itemManager);
    itemManager.Update(deltaTime, world, player, camera.x, camera.y, inventory);
    inventory.Update(player);

    camera.Follow(
        floorf(player.x),
        floorf(player.y),
        deltaTime,
        GetScreenWidth(),
        GetScreenHeight(),
        worldPixelWidth,
        worldPixelHeight
    );
}

void Game::Draw() {
    int camDrawX = (int)floor(camera.x);
    int camDrawY = (int)floor(camera.y);

    world.Render(camDrawX, camDrawY, GetScreenWidth(), GetScreenHeight(), textrueManager);
    player.Draw(camDrawX, camDrawY);
    itemManager.Render(camDrawX, camDrawY, textrueManager);
    editor.DrawHighlight(player);

    // UI
    player.DrawUI();
    inventory.Draw();
}

void Game::Destroy() {
    textrueManager.Unload();
}
