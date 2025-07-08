#include "blockEditor.hpp"

BlockEditor::BlockEditor(World& world, const GameCamera& camera)
    : world(world), camera(camera) {}

void BlockEditor::Update(Player& player) {
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        int tileX, tileY;
        if (GetHoveredTile(tileX, tileY)) {
            double dist = findDistance(tileX, tileY, player.x / tileSize, player.y / tileSize);
            if (dist <= blockReach) {
                if (tileX >= 0 && tileX < world.getWidth() &&
                    tileY >= 0 && tileY < world.getHeight()) {
                    world.at(tileX, tileY) = World::TILE_AIR;
                }
            }
        }
    }
}

void BlockEditor::DrawHighlight(Player& player) const {
    int tileX, tileY;
    if (GetHoveredTile(tileX, tileY)) {
        double dist = findDistance(tileX, tileY, player.x / tileSize, player.y / tileSize);
        if (dist <= blockReach) {
            if (world.IsSolidTile(tileX, tileY)) {
                DrawRectangle(
                    tileX * tileSize - camera.drawX,
                    tileY * tileSize - camera.drawY,
                    tileSize,
                    tileSize,
                    Fade(YELLOW, 0.3f)
                );

                DrawRectangleLines(
                    tileX * tileSize - camera.drawX,
                    tileY * tileSize - camera.drawY,
                    tileSize,
                    tileSize,
                    YELLOW
                );
            }
        }
    }
}

bool BlockEditor::GetHoveredTile(int& outTileX, int& outTileY) const {
    Vector2 mouse = GetMousePosition();

    float worldX = mouse.x + camera.drawX;
    float worldY = mouse.y + camera.drawY;

    outTileX = (int)(worldX / tileSize);
    outTileY = (int)(worldY / tileSize);

    return true;
}

