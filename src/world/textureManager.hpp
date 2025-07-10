#pragma once
#include <raylib.h>
#include <string>

class TextureManager {
public:
    void Load();
    void Unload();

    void WorldTextureManager(int tile, int camX, int camY, int tilePixelX, int tilePixelY);
    void ItemTextureManager(int tile, int camX, int camY, int xPos, int yPox, int size, float hover);
private:
    void m_LoadFilterTexture(const std::string& path, Texture2D& texture );
    void m_RenderBlock(Texture2D& texture, int tilePixelX, int tilePixelY, int camX, int camY);
    void m_RenderDroppedItem(Texture2D& texture, int xPos, int yPos, int camX, int camY, int size, float hover);

    Texture2D stoneTexture;
    Texture2D dirtTexture;
};
