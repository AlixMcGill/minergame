#pragma once
#include <raylib.h>

class HotBar {
public:
    int slots = 5;
    int slotSize = 5;
    int margin = 20;

    void Init();
    void Update();
    void Draw();
private:
};
