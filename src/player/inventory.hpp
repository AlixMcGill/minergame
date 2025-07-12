#pragma once

#include <cstdint>
#include <vector>
#include "../game/item.hpp"

class Player;
class ItemManager;

class Inventory { // handles player inventory
public:
    std::vector<Item> inventory;
    bool isInventoryOpen = false;
    bool overburdened = false;
    float currentWeight = 0;
    float maxWeight = 600;
    int hoveredItemIndex = -1;
    int dropRepeatCounter = 0;
    int dropRepeatDelay = 10;
   
    void Update(Player& player);
    void Draw();
    bool IsInventoryFull();
    void AddItemToInventory(Item& item);
    void RemoveItemById(uint64_t itemId, Player& player);
    void ScrollUp();
    void ScrollDown();
    void DropOneItemAtGroupedIndex(int groupedIndex, Player& player);
private:
    int scrollOffset = 0;
    const int maxVisibleLines = 20;
    const int lineHeight = 20;
};
