#include "raylib.h"
#include "drawingUtils.hpp"

#include <iostream>

void DrawChat(int screenheight, int screenwidth, int chatTextsize, std::string& chatContent) {
    DrawRectangle(10, screenheight - 25 - chatTextsize, screenwidth - 30, chatTextsize + 10, BLACK);
    DrawText(chatContent.c_str(), 15, screenheight - 15 - chatTextsize, chatTextsize, RAYWHITE);
}

void DrawMenu(int screenheight, int screenwidth, int buttonXPlay, int buttonYPlay, int buttonYOptions, 
    int buttonYQuit, int buttonWidth, int buttonHeight, Color backgroundColor, Texture2D background, Color color) {
    BeginDrawing();
    ClearBackground(backgroundColor);
    DrawTexture(background, 0, 0, WHITE);
    DrawButton(buttonXPlay, buttonYPlay, buttonWidth, buttonHeight, WHITE, color, "Play");
    DrawButton(buttonXPlay, buttonYOptions, buttonWidth, buttonHeight, WHITE, color, "Options");
    DrawButton(buttonXPlay, buttonYQuit, buttonWidth, buttonHeight, WHITE, color, "Quit Game");
    DrawText("Nyx", screenwidth / 2 - 40, screenheight - 3 * screenheight / 4, 40, WHITE);
    DrawText("Nyx, open-source video game project", screenwidth / 2 - screenwidth / 3 / 2, screenheight - screenheight / 4, 20, WHITE);
    EndDrawing();
}

void DrawPauseScreen(int screenheight, int screenwidth) {
    DrawText("Game paused", screenwidth / 2 - MeasureText("Game paused", 50) / 2, screenheight / 2 -  25, 50, RAYWHITE);
}