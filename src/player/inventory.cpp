#include "inventory.hpp"
#include <algorithm>
#include <cstdint>
#include <raylib.h>
#include <sstream>
#include <unordered_map>
#include "../player/player.hpp"
#include "../game/itemManager.hpp"


void Inventory::Update(Player& player) {
    if (IsKeyPressed(KEY_B)) { // inventory toggle
        isInventoryOpen = !isInventoryOpen;
    }

    float wheel = GetMouseWheelMove();
    if (wheel > 0) {
        ScrollUp();
    } else if (wheel < 0) {
        ScrollDown();
    }

    if (!isInventoryOpen || hoveredItemIndex == -1) {
        dropRepeatCounter = 0;
        return;
    }

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        DropOneItemAtGroupedIndex(hoveredItemIndex, player);
        dropRepeatCounter = 0;
    }

    if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        dropRepeatCounter++;
        if (dropRepeatCounter >= dropRepeatDelay) {
            DropOneItemAtGroupedIndex(hoveredItemIndex, player);
            dropRepeatCounter = 0;
        }
    }
}

void Inventory::Draw() {
    if (!isInventoryOpen) return;

    int panelX = 10;
    int panelY = 50;
    int panelWidth = 250;
    int panelHeight = maxVisibleLines * lineHeight + 30;

    // Draw background panel
    DrawRectangle(panelX, panelY, panelWidth, panelHeight, Fade(BLACK, 0.7f));
    DrawRectangleLines(panelX, panelY, panelWidth, panelHeight, DARKGRAY);

    int x = panelX + 10;
    int y = panelY + 20 - scrollOffset;  // apply scroll offset here

    DrawText("Inventory:", x, panelY + 5, 20, YELLOW);

    if (inventory.empty()) {
        DrawText("  (Empty)", x, y, 20, LIGHTGRAY);
        return;
    }

    // Group items by name
    std::unordered_map<std::string, int> groupedCounts;
    for (const auto& item : inventory) {
        groupedCounts[std::string(item.name)]++;
    }

    // Draw only visible lines
    int lineIndex = 0;
    hoveredItemIndex = -1;

    for (const auto& [name, count] : groupedCounts) {
        int lineY = y + lineIndex * lineHeight;

        // Skip off-screen items
        if (lineY + lineHeight < panelY + 20) {
            lineIndex++;
            continue;
        }
        if (lineY > panelY + panelHeight - 40) break;

        // Draw item text
        std::stringstream ss;
        ss << "- " << name << " x" << count;
        DrawText(ss.str().c_str(), x, lineY, 20, WHITE);

        // Draw Drop button
        Rectangle dropBtn = { (float)(panelX + panelWidth - 60), (float)lineY, 50.0f, (float)(lineHeight - 4) };
        DrawRectangleRec(dropBtn, GRAY);
        DrawRectangleLinesEx(dropBtn, 1, DARKGRAY);
        DrawText("Drop", dropBtn.x + 5, dropBtn.y + 2, 18, BLACK);

        // Mouse hover effect
        if (CheckCollisionPointRec(GetMousePosition(), dropBtn)) {
            hoveredItemIndex = lineIndex;
            DrawRectangleRec(dropBtn, Fade(WHITE, 0.2f)); // subtle hover
        }

        lineIndex++;
    }

    // Draw weight at the bottom
    int weightY = panelY + panelHeight - 25;
    char weightText[64];
    snprintf(weightText, sizeof(weightText), "Weight: %.0f / %.0f", currentWeight, maxWeight);
    Color weightColor = (currentWeight > maxWeight) ? RED : GREEN;
    DrawText(weightText, panelX + 10, weightY, 20, weightColor);
}


bool Inventory::IsInventoryFull() {
    if ( currentWeight >= maxWeight) {
        overburdened = true;
        return overburdened;
    } else {
        overburdened = false;
        return overburdened;
    }
}

void Inventory::AddItemToInventory(Item& item) {
    inventory.push_back(item);
    currentWeight += item.itemWeight;
}

void Inventory::RemoveItemById(uint64_t itemId, Player& player) {
    auto it = std::find_if(inventory.begin(), inventory.end(),
        [itemId](const Item& item) {
            return item.id == itemId;
        });

    if (it != inventory.end()) {
        currentWeight -= it->itemWeight;

        it->xPos = player.x + 30;
        it->yPos = player.y;
        it->location = Item::DROPPED;
        it->vx = 0;
        it->vy = 0;

        ItemManager::AddItemToWorld(std::move(*it));

        inventory.erase(it);
    }
}

void Inventory::ScrollUp() {
    scrollOffset -= lineHeight;
    if (scrollOffset < 0) scrollOffset = 0;
}

void Inventory::ScrollDown() {
    int totalLines = 0;

    // Count unique items (grouped)
    std::unordered_map<std::string, int> groupedCounts;
    for (const auto& item : inventory) {
        groupedCounts[std::string(item.name)]++;
    }
    totalLines = (int)groupedCounts.size();

    int maxOffset = std::max(0, (totalLines - maxVisibleLines) * lineHeight);

    scrollOffset += lineHeight;
    if (scrollOffset > maxOffset) scrollOffset = maxOffset;
}

void Inventory::DropOneItemAtGroupedIndex(int groupedIndex, Player& player) {
    // Group by name and map indices
    std::unordered_map<std::string, std::vector<size_t>> nameToIndices;
    for (size_t i = 0; i < inventory.size(); ++i) {
        nameToIndices[inventory[i].name].push_back(i);
    }

    // Get the name of the groupedIndex-th unique item
    std::vector<std::string> names;
    for (const auto& [name, _] : nameToIndices)
        names.push_back(name);

    if (groupedIndex < 0 || groupedIndex >= (int)names.size()) return;

    std::string nameToDrop = names[groupedIndex];
    auto& indices = nameToIndices[nameToDrop];

    if (!indices.empty()) {
        size_t dropIndex = indices.back();
        Item dropItem = inventory[dropIndex];
        
        int place = 30;
        int xspeed = 300;
        if (player.faceDir) {
            place = 30;
            xspeed = 300;
        } else {
            place = -30;
            xspeed = -300;
        }

        // Adjust inventory state
        currentWeight -= dropItem.itemWeight;
        dropItem.xPos = player.x + 30;
        dropItem.yPos = player.y;
        dropItem.vx = 800;
        dropItem.vy = -300;
        dropItem.location = Item::DROPPED;

        ItemManager::AddItemToWorld(std::move(dropItem));
        inventory.erase(inventory.begin() + dropIndex);
    }
}

