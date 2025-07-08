#pragma once

#include <algorithm>  // for std::clamp
#include <cmath>      // for std::floor

struct GameCamera {
    float x = 0;
    float y = 0;
    float speed = 1800.0f;

    int drawX = 0; // Integer-aligned position for rendering
    int drawY = 0;

    void Follow(float targetX, float targetY, float deltaTime, int screenWidth, int screenHeight, int worldPixelWidth, int worldPixelHeight);
};

