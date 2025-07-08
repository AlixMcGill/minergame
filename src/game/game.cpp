#include "game.hpp"

Game::Game()
    : editor(world, camera) // initialize editor
{
    world.GenerateTerrain();

    player.x = 200;
    player.y = 200;

    worldPixelWidth = world.getWidth() * tileSize;
    worldPixelHeight = world.getHeight() * tileSize;
}

void Game::Update(float deltaTime) {
    player.Update(deltaTime, world);
    editor.Update(player);

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

    world.Render(camDrawX, camDrawY, GetScreenWidth(), GetScreenHeight());
    player.Draw(camDrawX, camDrawY);
    editor.DrawHighlight(player);
}

