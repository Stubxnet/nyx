#include "raylib.h"
#include "drawingUtils.hpp"
#include "../enum.hpp"

#include <iostream>

void DrawChat(int screenheight, int screenwidth, int chatTextsize, std::string& chatContent) {
    DrawRectangle(10, screenheight - 25 - chatTextsize, screenwidth - 30, chatTextsize + 10, BLACK);
    DrawText(chatContent.c_str(), 15, screenheight - 15 - chatTextsize, chatTextsize, RAYWHITE);
}

MenuAction DrawAndHandleMenu(int screenHeight, int screenWidth, Texture2D background, Color bgColor, Color buttonColor) {
    const float buttonWidth = 200;
    const float buttonHeight = 50;
    const float buttonSpacing = 20;

    float buttonXPlay = screenWidth/2 - buttonWidth/2;
    float buttonYPlay = screenHeight/2 - buttonHeight - 20;
    float buttonYOptions = buttonYPlay + buttonHeight + buttonSpacing;
    float buttonYQuit = buttonYPlay + 2*(buttonHeight + buttonSpacing);

    BeginDrawing();
    ClearBackground(bgColor);
    DrawTexture(background, 0, 0, WHITE);
    DrawButton(buttonXPlay, buttonYPlay, buttonWidth, buttonHeight, WHITE, buttonColor, "Play");
    DrawButton(buttonXPlay, buttonYOptions, buttonWidth, buttonHeight, WHITE, buttonColor, "Options");
    DrawButton(buttonXPlay, buttonYQuit, buttonWidth, buttonHeight, WHITE, buttonColor, "Quit Game");
    DrawText("Nyx", screenWidth/2 - 40, screenHeight - 3*screenHeight/4, 40, WHITE);
    DrawText("Nyx, open-source video game project", screenWidth/2 - screenWidth/3/2, screenHeight - screenHeight/4, 20, WHITE);
    EndDrawing();

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (CheckMousePosition(buttonXPlay, buttonYPlay, buttonWidth, buttonHeight)) {
            return MenuAction::PLAY;
        } else if (CheckMousePosition(buttonXPlay, buttonYOptions, buttonWidth, buttonHeight)) {
            return MenuAction::OPTIONS;
        } else if (CheckMousePosition(buttonXPlay, buttonYQuit, buttonWidth, buttonHeight)) {
            return MenuAction::QUIT;
        }
    }
    return MenuAction::NONE;
}

OptionsAction DrawAndHandleOptions(int screenWidth, int screenHeight, Texture2D background, Color bgColor, Color buttonColor) {    
    const float buttonWidth = 200;    
    const float buttonHeight = 50;    
    const float buttonSpacing = 20;
    float totalButtonWidth = 2 * buttonWidth + buttonSpacing;    
    float buttonX1 = (screenWidth - totalButtonWidth) / 2.0f;    
    float buttonX2 = buttonX1 + buttonWidth + buttonSpacing;    
    float buttonY  = screenHeight - buttonHeight - 20.0f;
    BeginDrawing();    
    ClearBackground(bgColor);    
    DrawTexture(background, 0, 0, WHITE);    
    DrawButton(buttonX1, buttonY, buttonWidth, buttonHeight, WHITE, buttonColor, "Quit Game");    
    DrawButton(buttonX2, buttonY, buttonWidth, buttonHeight, WHITE, buttonColor, "Back to menu");    
    EndDrawing();
    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {        
        if (CheckMousePosition(buttonX1, buttonY, buttonWidth, buttonHeight)) {            
            return OptionsAction::QUIT;        
        } else if (CheckMousePosition(buttonX2, buttonY, buttonWidth, buttonHeight)) {            
            return OptionsAction::BACK;        
        }    
    }    
    return OptionsAction::NONE;
}

void DrawPauseScreen(int screenheight, int screenwidth) {
    DrawText("Game paused", screenwidth / 2 - MeasureText("Game paused", 50) / 2, screenheight / 2 -  25, 50, RAYWHITE);
}