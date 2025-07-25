#include "player.hpp"
#include <cmath>
#include <algorithm>
#include <iostream>
#include <raylib.h>

extern int tileSize;

void Player::Init(const World& world) {
    FindSpawn(world);
    x = spawnX;
    y = spawnY;
    vy = 0;
    vx = 0;
    health = maxHealth;
    isDead = false;
    isOnGround = true;
    ignoreFallDamage = true;
    fallDamageTimer = 0.0;
    StartFallDamageTimer(1.5f);
}

void Player::Update(float deltaTime, const World& world) {
    IgnoreFallDamage(deltaTime);

    if (isDead && IsKeyPressed(KEY_R)) {
        Respawn();
    } else if (isDead) {
        return; // causes all player updates to not function must be at top
    }
    if (IsKeyPressed(KEY_R)) Respawn(); // For Testing Only
    
    vx = 0;
    if (IsKeyDown(KEY_A)) {
        faceDir = false;
        vx = -speed;
    }
    if (IsKeyDown(KEY_D)) {
        faceDir = true;
        vx = speed;
    }

    if ((isOnGround || IsOnGroundWithTolerance(world)) && IsKeyPressed(KEY_SPACE)) {
        vy = -600.0f;
        isOnGround = false;
    }

    vy += gravity * deltaTime;
    if (vy > maxFallSpeed) vy = maxFallSpeed;

    MoveX(vx * deltaTime, world);
    MoveY(vy * deltaTime, world);
    //std::cout << "x: " << x << " y: " << y << " vx: " << vx << " vy: " << vy << std::endl;
}

void Player::MoveX(float dx, const World& world) {
    float sign = dx > 0 ? 1.0f : -1.0f;
    float step = tileSize / 4.0f;
    float moved = 0.0f;

    while (std::abs(moved) < std::abs(dx)) {
        float move = std::min(step, std::abs(dx - moved)) * sign;
        x += move;
        if (IsColliding(world)) {
            x -= move;
            vx = 0;
            break;
        }
        moved += move;
    }
}

void Player::MoveY(float dy, const World& world) {
    float sign = dy > 0 ? 1.0f : -1.0f;
    float step = tileSize / 4.0f;
    float moved = 0.0f;

    float preCollisionVy = vy;

    while (std::abs(moved) < std::abs(dy)) {
        float move = std::min(step, std::abs(dy - moved)) * sign;
        y += move;
        if (IsColliding(world)) {
            y -= move;

            // Fall Damage
            if (dy > 0 && preCollisionVy > safeFallSpeed && !ignoreFallDamage) {
                int damage = static_cast<int>((preCollisionVy - safeFallSpeed) * fallSpeedDamageScale);
                if (preCollisionVy > (safeFallSpeed * 2)) damage = maxHealth;
                TakeDamage(damage);
            }

            vy = 0;
            if (dy > 0)
                isOnGround = true;
            return;
        }
        moved += move;
    }

    isOnGround = false;
}

void Player::IgnoreFallDamage(float deltaTime) {
    if (ignoreFallDamageTimerStarted && ignoreFallDamage) {
        std::cout << "FallDamageTimer: " << fallDamageTimer << std::endl;
        fallDamageTimer += deltaTime;
        if (fallDamageTimer >= timeToIgnoreFallDamage) {
            ignoreFallDamage = false;
            ignoreFallDamageTimerStarted = false;
        }
    }
}

void Player::StartFallDamageTimer(float time) {
    ignoreFallDamage = true;
    ignoreFallDamageTimerStarted = true;
    timeToIgnoreFallDamage = time;
    fallDamageTimer = 0.0f;
}

void Player::TakeDamage(int damage) {
    std::cout << "Took Damage" << damage << std::endl;
    health -= damage;
    if (health <= 0) {
        health = 0;
        isDead = true;
    }
}

void Player::Respawn() {
    x = spawnX;
    y = spawnY;
    vx = vy = 0;
    isDead = false;
    isOnGround = true;
    health = maxHealth;
    StartFallDamageTimer(2.0f);
}

bool Player::IsOnGroundWithTolerance(const World& world, float tolerance) const {
    return IsCollidingAt(x, y + height + tolerance, width, 0.1f, world);
}

bool Player::IsCollidingAt(float px, float py, float w, float h, const World& world) const {
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

bool Player::IsColliding(const World& world) const {
    return IsCollidingAt(x, y, width, height, world);
}

void Player::Draw(int camDrawX, int camDrawY) const {
    Color playerColor;
    if (ignoreFallDamage) {
        playerColor = GREEN; 
    } else {
        playerColor = RED;
    }
    DrawRectangle((int)(x - camDrawX), (int)(y - camDrawY), (int)width, (int)height, playerColor);
}

void Player::DrawUI() {
    const int barWidth = 200;
    const int barHeight = 20;
    const int margin = 20;

    int sw = GetScreenWidth();
    //int sh = GetScreenHeight();

    // top right position
    int hBarX = sw - barWidth - margin;
    int hBarY = margin;

    // Health Bar
    float healthPercent = (float)health / maxHealth;
    DrawRectangle(hBarX, hBarY, barWidth, barHeight, DARKGRAY); // background
    DrawRectangle(hBarX, hBarY, (int)(barWidth * healthPercent), barHeight, RED); // Health
    DrawRectangleLines(hBarX, hBarY, barWidth, barHeight, BLACK); // border
    DrawText(TextFormat("HP: %d / %d", health, maxHealth), hBarX + (barWidth / 4), hBarY + (barHeight / 5), 14, WHITE);



    // Death Screen
    if (isDead) {
        DrawText("You died! Press R to respawn.", GetScreenWidth() / 2 - 100, GetScreenHeight() / 3, 20, RED);
    }
}

void Player::DebugDrawBounds(const GameCamera& cam) const {
    DrawRectangleLines(
        (int)(x - cam.x),
        (int)(y - cam.y),
        (int)width,
        (int)height,
        GREEN
    );
}

void Player::FindSpawn(const World& world) {
    int middleX = world.getWidth() / 2;

    for (int y = 0; y < world.getHeight(); y++) {
        if (world.IsSolidTile(middleX, y)) {
            spawnX = middleX * tileSize;
            spawnY = (y - 4) * tileSize;
            return;
        }
    }
}
