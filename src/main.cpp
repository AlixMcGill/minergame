#pragma once
#include "util/globals.hpp"
#include "game/game.hpp"

int main() {
    InitWindow(windowWidth, windowHeight, "Miner Game");
    SetTargetFPS(240);

    Game game;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        game.Update(deltaTime);

        BeginDrawing();
        ClearBackground(BLUE);

        BeginScissorMode(0, 0, GetScreenWidth(), GetScreenHeight());
        game.Draw();
        EndScissorMode();

        DrawFPS(10, 10);
        EndDrawing();
    }

    CloseWindow();
    return 0;
}

