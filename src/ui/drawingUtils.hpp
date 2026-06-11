#include "raylib.h"
#pragma once

void DrawButton(float x, float y, float width, float height, Color textColor, Color buttonColor, const char* text, const Font& font = GetFontDefault()) {
    DrawRectangle(x, y, width, height, buttonColor);

    Vector2 textSize = MeasureTextEx(font, text, height * 0.4f, 1.0f);

    Vector2 textPosition = {
        x + (width - textSize.x) / 2,
        y + (height - textSize.y) / 2
    };

    DrawTextEx(font, text, textPosition, height * 0.4f, 1.0f, textColor);
}
