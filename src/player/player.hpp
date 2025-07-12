#pragma once

#include "../world/world.hpp"
#include "camera.hpp"
#include <raylib.h>

class Player {
public:
    float spawnX = 800;
    float spawnY = 0;
    float x = 0;
    float y = 0;
    float width = 22;
    float height = 44;

    bool faceDir = false;

    int maxHealth = 100;
    int health = 100;
    bool isDead = false;

    float speed = 300.0f;
    float vx = 0;
    float vy = 0;

    float gravity = 1200.0f;
    float maxFallSpeed = 10000.0f;
    float safeFallSpeed = 1000.0f;
    float fallSpeedDamageScale = 0.2f;

    bool isOnGround = true;

    int itemPickupDistance = 100; // distance to when the item starts to move toward the player
    int itemGrabDistance = 30; // distance to when the item moves into player inventory
    bool inventoryFull = false;

    void Init(const World& world);
    void Update(float deltaTime, const World& world);
    void MoveX(float dx, const World& world);
    void MoveY(float dy, const World& world);
    void TakeDamage(int damage);
    void Respawn();

    bool IsOnGroundWithTolerance(const World& world, float tolerance = 10.0f) const;
    bool IsCollidingAt(float px, float py, float w, float h, const World& world) const;
    bool IsColliding(const World& world) const;

    void Draw(int camDrawX, int camDrawY) const;
    void DrawUI();
    void DebugDrawBounds(const GameCamera& cam) const;
    void FindSpawn(const World& world);
};
