#include "game.hpp"

Game::Game()
    : editor(world, camera) // initialize editor
{
    textrueManager.Load();
    world.GenerateTerrain();
    player.Init();

    worldPixelWidth = world.getWidth() * tileSize;
    worldPixelHeight = world.getHeight() * tileSize;
}

void Game::Update(float deltaTime) {
    player.Update(deltaTime, world);
    editor.Update(player, itemManager);
    itemManager.Update(deltaTime, world, player, camera.x, camera.y);

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
}

void Game::Destroy() {
    textrueManager.Unload();
}
