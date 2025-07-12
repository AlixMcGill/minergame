#pragma once
#include "item.hpp"
#include "../world/world.hpp"
#include <cstdint>
#include <raylib.h>
#include <unordered_set>
#include <vector>
#include <algorithm>

// manages all items in world
//
class Inventory;

class ItemManager {
public:
    static std::vector<Item> items;

    void CreateDroppedItem(const char* name, float x, float y, int itemWeight, Item::ItemRenderType textrue);
    static void AddItemToWorld(Item&& item);
    void Update(float deltaTime, World& world, Player& player, int camX, int camY, Inventory& inventory);
    void Render(float camX, float camY, TextureManager& textureManager);
private:
    bool PickupItem(Item& item, Player& player, Inventory& inventory);
    uint64_t m_GenerateUniqueId();
    void m_RemoveId(uint64_t id);
    void m_RemoveItemFromWorld(uint64_t id);
};
