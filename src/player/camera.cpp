#include "camera.hpp"

void GameCamera::Follow(float targetX, float targetY, float deltaTime, int screenWidth, int screenHeight, int worldPixelWidth, int worldPixelHeight) {
    float targetCamX = std::floor(targetX - screenWidth / 2);
    float targetCamY = std::floor(targetY - screenHeight / 2);

    float lerpSpeed = 8.0f;

    x += (targetCamX - x) * lerpSpeed * deltaTime;
    y += (targetCamY - y) * lerpSpeed * deltaTime;

    // Clamp camera
    x = std::clamp(x, 0.0f, std::max(0.0f, (float)(worldPixelWidth - screenWidth)));
    y = std::clamp(y, 0.0f, std::max(0.0f, (float)(worldPixelHeight - screenHeight)));

    // Snap for rendering
    drawX = (int)std::floor(x);
    drawY = (int)std::floor(y);
}

