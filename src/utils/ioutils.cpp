#include "raylib.h"

bool CheckMousePosition(int buttonX, int buttonY, int buttonWidth, int buttonHeight) {
    Vector2 mousePoint = { (float)GetMouseX(), (float)GetMouseY() };

    return (mousePoint.x >= buttonX && mousePoint.x <= buttonX + buttonWidth && mousePoint.y >= buttonY && mousePoint.y <= buttonY + buttonHeight);
}
