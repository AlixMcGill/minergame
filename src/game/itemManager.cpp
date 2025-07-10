#include "itemManager.hpp"
#include "item.hpp"
#include <cstring>
#include <iostream>
#include <ostream>
#include <raylib.h>

void ItemManager::CreateDroppedItem(const char* name, float x, float y, Item::ItemRenderType texture) {
    Item item;
    std::strncpy(item.name, name, sizeof(item.name));
    item.name[sizeof(item.name) - 1] = '\0';
    item.xPos = x + (tileSize / 2) - (item.size / 2);
    item.yPos = y;

    item.location = Item::DROPPED;
    item.texture = texture;

    items.push_back(item);
}

void ItemManager::Update(float deltaTime, World& world, Player& player, int camX, int camY) {
    for (auto& item : items) {
        if (item.xPos < camX + GetScreenWidth() && item.xPos > camX && item.yPos > camY && item.yPos < camY + GetScreenHeight()) {
            if (item.location == Item::DROPPED) {
                item.UpdateDropped(deltaTime, world, player);
                PickupItem(item, player);
            }
        }
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

void ItemManager::PickupItem(Item& item, Player& player) {
    if (item.distanceToPlayer <= player.itemGrabDistance && player.itemInventory < player.itemMaxInventory) {
        item.location = Item::INVENTROY;
        player.itemInventory++;
    }
    std::cout << player.itemInventory << std::endl;
}
