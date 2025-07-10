#pragma once
#include "item.hpp"
#include "../world/world.hpp"
#include <raylib.h>
#include <vector>

// manages all items in world

class ItemManager {
public:
    std::vector<Item> items;

    void CreateDroppedItem(const char* name, float x, float y, Item::ItemRenderType textrue);
    void Update(float deltaTime, World& world, Player& player);
    void Render(float camX, float camY, TextureManager& textureManager);
private:
    void PickupItem(Item& item, Player& player);
};
