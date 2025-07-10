#include "textureManager.hpp"
#include "world.hpp" // for texture types
#include "../game/item.hpp" // for texture types
#include <raylib.h>

void TextureManager::Load() {
    m_LoadFilterTexture("graphics/blocks/Stone.png", stoneTexture);
    m_LoadFilterTexture("graphics/blocks/Dirt.png", dirtTexture);
}

void TextureManager::Unload() {
    UnloadTexture(stoneTexture);
    UnloadTexture(dirtTexture);
}

void TextureManager::WorldTextureManager(int tile, int camX, int camY, int tilePixelX, int tilePixelY){
    switch (tile) {
        case World::TILE_STONE: 
            m_RenderBlock(stoneTexture, tilePixelX, tilePixelY, camX, camY);
            break;
        case World::TILE_DIRT: 
            m_RenderBlock(dirtTexture, tilePixelX, tilePixelY, camX, camY);
            break;
    }
}

void TextureManager::ItemTextureManager(int tile, int camX, int camY, int xPos, int yPos, int size, float hover){
    switch (tile) {
        case Item::BLOCK_STONE: 
            m_RenderDroppedItem(stoneTexture, xPos, yPos, camX, camY, size, hover);
            break;
        case Item::BLOCK_DIRT: 
            m_RenderDroppedItem(dirtTexture, xPos, yPos, camX, camY, size, hover);
            break;
    }
}

void TextureManager::m_RenderBlock(Texture2D& texture, int tilePixelX, int tilePixelY, int camX, int camY) {
    Rectangle src = {0.0f, 0.0f, (float)texture.width, (float)texture.height};
    Rectangle dest = {(float)(tilePixelX - camX), (float)(tilePixelY - camY), (float)tileSize, (float)tileSize};
    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(texture, src, dest, origin, 0.0f, WHITE);
}

void TextureManager::m_RenderDroppedItem(Texture2D& texture, int xPos, int yPos, int camX, int camY, int size, float hover) {
    int offset = 2;
    DrawRectangle((int)(xPos - camX) - offset + hover, (int)(yPos- camY) + offset + hover, (int)size, (int)size, BLACK);
    Rectangle src = {0.0f, 0.0f, (float)texture.width, (float)texture.height};
    Rectangle dest = {(float)(xPos - camX) + hover, (float)(yPos - camY) + hover, (float)size, (float)size};
    Vector2 origin = {0.0f, 0.0f};
    DrawTexturePro(texture, src, dest, origin, 0.0f, WHITE);
}

void TextureManager::m_LoadFilterTexture(const std::string& path, Texture2D& texture) {
    texture = LoadTexture(path.c_str());

    if (texture.id == 0) {
        TraceLog(LOG_ERROR, "Failed to load texture: %s", path.c_str());
        return;
    }
    SetTextureFilter(texture, TEXTURE_FILTER_POINT);
}
