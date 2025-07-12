#include "itemManager.hpp"
#include "item.hpp"
#include "../player/inventory.hpp"
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <ostream>
#include <random>
#include <raylib.h>
#include <utility>

static std::unordered_set<uint64_t> usedIds;
std::vector<Item> ItemManager::items;

void ItemManager::CreateDroppedItem(const char* name, float x, float y, int itemWeight, Item::ItemRenderType texture) {
    Item item;
    std::strncpy(item.name, name, sizeof(item.name));
    item.name[sizeof(item.name) - 1] = '\0';
    item.xPos = x + (tileSize / 2) - (item.size / 2);
    item.yPos = y;
    item.itemWeight = itemWeight;
    item.location = Item::DROPPED;
    item.texture = texture;
    item.id = m_GenerateUniqueId();
    std::cout << item.id << std::endl;

    items.push_back(item);
}

void ItemManager::Update(float deltaTime, World& world, Player& player, int camX, int camY, Inventory& inventory) {
    std::vector<uint64_t> itemsToRemove;

    for (auto& item : items) {
        if (item.xPos < camX + GetScreenWidth() && item.xPos > camX && item.yPos > camY && item.yPos < camY + GetScreenHeight()) {
            if (item.location == Item::DROPPED) {
                item.UpdateDropped(deltaTime, world, player);
                if (PickupItem(item, player, inventory)) {
                    itemsToRemove.push_back(item.id);
                }
            }
        }
    }

    for (auto id : itemsToRemove) {
        m_RemoveItemFromWorld(id);
    }
}

void ItemManager::Render(float camX, float camY, TextureManager& textureManager) { // camerax and y
    for (auto& item : items) {
        if (item.xPos < camX + GetScreenWidth() && item.xPos > camX && item.yPos > camY && item.yPos < camY + GetScreenHeight()) {
            if (item.location == Item::DROPPED) {
                item.RenderDropped(camX, camY, textureManager);
            }
        }
    }
}

bool ItemManager::PickupItem(Item& item, Player& player, Inventory& inventory) {
    player.inventoryFull = inventory.IsInventoryFull();
    if (item.distanceToPlayer <= player.itemGrabDistance && !inventory.IsInventoryFull()) {
        item.location = Item::INVENTORY;
        inventory.AddItemToInventory(item);
        return true;
    }
    return false;
}

uint64_t ItemManager::m_GenerateUniqueId() {
    static std::mt19937_64 rng(std::random_device{}());
    static std::uniform_int_distribution<uint64_t> dist;

    uint64_t id;
    do {
        id = dist(rng);
    } while (usedIds.count(id));

    usedIds.insert(id);
    return id;
}

void ItemManager::m_RemoveId(uint64_t id) {
    usedIds.erase(id);
}

void ItemManager:: AddItemToWorld(Item&& item) {
    items.push_back(std::move(item));
}

void ItemManager::m_RemoveItemFromWorld(uint64_t id) {
    items.erase(
            std::remove_if(items.begin(), items.end(),
                [id](const Item& item) {
                    return item.id == id;
                }),
                items.end()
            );
}
