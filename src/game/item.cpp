#include "item.hpp"
#include <cmath>
#include <iostream>
#include <ostream>
#include <raylib.h>
#include "../util/utils.hpp"

void Item::UpdateDropped(float deltaTime, World& world, Player& player) {
    vy += gravity * deltaTime;
    if (vy > maxFallSpeed) vy = maxFallSpeed;

    MoveX(vx * deltaTime, world);
    MoveY(vy * deltaTime, world);
    MoveDroppedTowardPlayer(deltaTime, player);
    ApplyFriction(deltaTime);
}

void Item::RenderDropped(float camDrawX, float camDrawY, TextureManager& textureManager) {
    textureManager.ItemTextureManager(texture, camDrawX, camDrawY, xPos, yPos, size, m_HoverAnimation());
}

bool Item::IsCollidingAt(float px, float py, float w, float h, const World& world) const {
    int tileX0 = (int)floor(px / tileSize);
    int tileY0 = (int)floor(py / tileSize);
    int tileX1 = (int)floor((px + w - 0.01f) / tileSize);
    int tileY1 = (int)floor((py + h - 0.01f) / tileSize);

    for (int ty = tileY0; ty <= tileY1; ++ty) {
        for (int tx = tileX0; tx <= tileX1; ++tx) {
            if (world.IsSolidTile(tx, ty)) return true;
        }
    }
    return false;
}

bool Item::IsColliding(const World& world) const {
    return IsCollidingAt(xPos, yPos, size, size, world);
}

void Item::MoveX(float dx, const World& world) {
    float sign = dx > 0 ? 1.0f : -1.0f;
    float step = tileSize / 4.0f;
    float moved = 0.0f;

    while (std::abs(moved) < std::abs(dx)) {
        float move = std::min(step, std::abs(dx - moved)) * sign;
        xPos += move;
        if (IsColliding(world)) {
            xPos -= move;
            vx = 0;
            break;
        }
        moved += move;
    }
}

void Item::MoveY(float dy, const World& world) {
    float sign = dy > 0 ? 1.0f : -1.0f;
    float step = tileSize / 4.0f;
    float moved = 0.0f;

    while (std::abs(moved) < std::abs(dy)) {
        float move = std::min(step, std::abs(dy - moved)) * sign;
        yPos += move;
        if (IsColliding(world)) {
            yPos -= move;
            vy = 0;
            break;
        }
        moved += move;
    }
}

void Item::MoveDroppedTowardPlayer(float deltaTime, Player& player) {
    if (!player.inventoryFull) {
        float moveSpeed = 0.2;
        distanceToPlayer = findDistance(player.x + (tileSize / 2), player.y + tileSize, xPos, yPos);
        Vector2 diff = diffVector(player.x, player.y, xPos, yPos);

        if (distanceToPlayer <= player.itemPickupDistance) {
            vx -= diff.x * moveSpeed;
            vy -= diff.y * moveSpeed;
        }
    }
}

void Item::ApplyFriction(float deltaTime) {
    if (vx > 0.0f) {
        vx -= friction * deltaTime;
    } else if (vx < 0.0f) {
        vx += friction * deltaTime;
    }
    if (vy > 0.0f) {
        vy -= friction * deltaTime;
    } else if (vy < 0.0f) {
        vy += friction * deltaTime;
    }
}

float Item::m_HoverAnimation() {
    float amp = 1.5f;
    float speed = 2.5f;
    float time = GetTime();

    return sinf(time * speed + phaseOffset) * amp;
}
