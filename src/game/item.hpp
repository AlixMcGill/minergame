#pragma once
#include <raylib.h>
#include "../world/world.hpp"
#include "../player/player.hpp"

// item class for definition of items

class Item {
public:
    char name[64];
    float xPos = 0.0f; // world coords if dropped on the ground
    float yPos = 0.0f;
    float speed = 300.0f;
    float friction = 350.0f; // friction
    float vx = 0;
    float vy = 0;
    float gravity = 1200.0f;
    float maxFallSpeed = 10000.0f;
    float size = 15.0f;
    double distanceToPlayer = 0;

    char chestID[64]; // if stored in a chest what chest its stored in
    int hotBarLocation;

    enum ItemLocation { // all states of an item
        DROPPED,
        PLACED_IN_WORLD,
        INVENTROY,
        HOTBAR,
        CHEST,
        DESTROY
    };

    ItemLocation location; // stores state of where the item is in the world
                           //
    enum ItemRenderType {
        BLOCK_STONE,
        BLOCK_DIRT
    };

    ItemRenderType texture;
                           
    bool IsCollidingAt(float px, float py, float w, float h, const World& world) const;
    bool IsColliding(const World& world) const;
    void MoveX(float dx, const World& world);
    void MoveY(float dy, const World& world);
    
    void UpdateDropped(float deltaTime, World& world, Player& player);
    void RenderDropped(float x, float y);
private:
    void MoveDroppedTowardPlayer(float deltaTime, Player& player);
    void ApplyFriction(float deltaTime);
};
