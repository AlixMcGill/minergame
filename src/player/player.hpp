#pragma once

#include "../world/world.hpp"
#include "camera.hpp"
#include <raylib.h>

class Player {
public:
    float x = 0;
    float y = 0;
    float width = 22;
    float height = 44;

    int maxHealth = 100;
    int health = 100;

    float speed = 300.0f;
    float vx = 0;
    float vy = 0;

    float gravity = 1200.0f;
    float maxFallSpeed = 1200.0f;
    float safeFallSpeed = 1000.0f;
    float fallSpeedDamageScale = 0.5f;

    bool isOnGround = false;

    void Update(float deltaTime, const World& world);
    void MoveX(float dx, const World& world);
    void MoveY(float dy, const World& world);
    void TakeDamage(int damage);

    bool IsOnGroundWithTolerance(const World& world, float tolerance = 10.0f) const;
    bool IsCollidingAt(float px, float py, float w, float h, const World& world) const;
    bool IsColliding(const World& world) const;

    void Draw(int camDrawX, int camDrawY) const;
    void DrawUI();
    void DebugDrawBounds(const GameCamera& cam) const;
};
