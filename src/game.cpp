#include "game.hpp"

enum GameScreen { MENU, GAME, OPTIONS };

const int CHUNK_SIZE = 16;

void Game::init() {
    // TODO: use this initialization function
}

void Game::run(const Config& config) {
    const char* title = config.windowTitle.c_str();
    InitWindow(config.windowWidth, config.windowHeight, title);

    GameScreen currentScreen = MENU;

    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    //////////////////////////////////////////////////////////////////////////
    /////////////////      WORLD CLASS INITIALIZATION        /////////////////
    //////////////////////////////////////////////////////////////////////////

    GameRules gamerules;
    
    World currentWorld("Default World", { 0.0f, 0.0f, 0.0f });

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {
                auto chunk = std::make_shared<Chunk>();
                currentWorld.AddChunk(chunk);

                for (int i = 0; i < 16; ++i) {
                    for (int j = 0; j < 16; ++j) {
                        for (int k = 0; k < 16; ++k) {
                            chunk->SetBlockId(i, j, k, 0);
                        }
                    }
                }
            }
        }
    }

    currentWorld.SetBlockId(2, 2, 2, 5);
    currentWorld.SetBlockId(2, 3, 2, 5);
    currentWorld.SetBlockId(2, 4, 2, 5);

    //////////////////////////////////////////////////////////////////////////
    /////////////////            TEXTURES LOADING            /////////////////
    //////////////////////////////////////////////////////////////////////////

    Texture2D background = LoadTexture(genPath(config.gameDirectory, 
                  "assets/textures/background/background1920x1080p.png").c_str());

    // This method to draw a block with models will probably used for falling blocks
    //Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    //Model cubeModel = LoadModelFromMesh(cubeMesh);

    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
    Image cubeTextureImage = LoadImage(genPath(config.gameDirectory, "assets/textures/blocks/textures5.png").c_str());
    Texture2D texture = LoadTextureFromImage(cubeTextureImage);

    //cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    UnloadImage(cubeTextureImage);

    //////////////////////////////////////////////////////////////////////////
    //////////////////                 CAMERA               //////////////////
    //////////////////////////////////////////////////////////////////////////

    Camera camera = { 0 };
    camera.position = (Vector3){ 0.0f, 2.0f, 4.0f };
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int cameraMode = CAMERA_FIRST_PERSON;

    float player_speed = 0.2f;                // TODO: Define a class Entity and create an instance for the player
    Vector3 rotation = {0.0f, 0.0f, 0.0f};

    float zoom = GetMouseWheelMove() * 0.5f;

    //-------------------------------------
    // Vars

    int renderDistance = 2;

    bool IsGamePaused = false;
    bool IsMouseEnabled = false;
    bool IsMovementsEnabled = true;

    // MENU screen
    float buttonWidth = 200;
    float buttonHeight = 50;
    float buttonSpacing = 20;

    //----------- HUD-related functions-------------

    // F3
    bool f3enabled = false;
    int f3textsize = GetRenderHeight() / 40;
    int f3textSpacing = f3textsize / 2;
    int f3lineSize = f3textSpacing + f3textsize;
    // gives space to DrawFPS
    f3textSpacing += 30;

    // F1
    int HideHUD = false;

    // Chat
    int IsChatOpened = false;
    std::string chatContent;

    while (!WindowShouldClose()) {
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyDown(KEY_C)) {
            std::cout << "Pressed Ctrl+C. Exiting." << std::endl;
            break;
        }
        
        switch (currentScreen) {
            case MENU: {
                float buttonXPlay = GetRenderWidth() / 2 - buttonWidth / 2;
                float buttonYPlay = GetRenderHeight() / 2 - buttonHeight - 20;
                float buttonYOptions = buttonYPlay + buttonHeight + buttonSpacing;
                float buttonYQuit = buttonYPlay + 2 * (buttonHeight + buttonSpacing);

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    if (CheckMousePosition(buttonXPlay, buttonYPlay, buttonWidth, buttonHeight)) {
                        std::cout << "Clicked Play" << std::endl;
                        currentScreen = GAME;
                    } else if (CheckMousePosition(buttonXPlay, buttonYOptions, buttonWidth, buttonHeight)) {
                        std::cout << "Clicked Options" << std::endl;
                        currentScreen = OPTIONS;
                    } else if (CheckMousePosition(buttonXPlay, buttonYQuit, buttonWidth, buttonHeight)) {
                        std::cout << "Clicked Quit Game" << std::endl;
                        UnloadTexture(texture);
                        UnloadTexture(background);
                        CloseWindow();
                    }
                }

                BeginDrawing();
                ClearBackground(backgroundColor);
                DrawTexture(background, 0, 0, WHITE);
                DrawButton(buttonXPlay, buttonYPlay, buttonWidth, buttonHeight, 5, WHITE, backgroundColor, WHITE, "Play");
                DrawButton(buttonXPlay, buttonYOptions, buttonWidth, buttonHeight, 5, WHITE, backgroundColor, WHITE, "Options");
                DrawButton(buttonXPlay, buttonYQuit, buttonWidth, buttonHeight, 5, WHITE, backgroundColor, WHITE, "Quit Game");
                DrawText("Nyx", GetRenderWidth() / 2 - 40, GetRenderHeight() - 3 * GetRenderHeight() / 4, 40, WHITE);
                DrawText("Nyx, open-source video game project", GetRenderWidth() / 2 - GetRenderWidth() / 3 / 2, GetRenderHeight() - GetRenderHeight() / 4, 20, WHITE);
                EndDrawing();
                break;
            }

            case GAME: {
                int key = GetCharPressed();

                if (IsKeyPressed(KEY_ESCAPE)) {
                    if (IsChatOpened) {
                        IsChatOpened = false;
                        chatContent = "";
                        std::cout << "Chat closed" << std::endl;
                    } else {
                        if (IsGamePaused) {
                            IsGamePaused = false;
                            std::cout << "Game restarted.";
                            DisableCursor();
                        } else if (!IsGamePaused) {
                            IsGamePaused = true;
                            std::cout << "Game paused.";
                            ShowCursor();
                        }
                    }
                }

                if (!IsGamePaused) {
                    rotation.x = GetMouseDelta().x * 0.1f;
                    rotation.y = GetMouseDelta().y * 0.1f;

                    Vector3 movement = {0.0f, 0.0f, 0.0f};
                    // ---------- Cursor management

                    if (!IsMouseEnabled) {
                        if (!IsCursorHidden()) {
                            DisableCursor();
                        }
                    }

                    if (IsChatOpened) {
                        if (IsKeyPressed(KEY_BACKSPACE)) {
                            if (!chatContent.empty()) {
                                chatContent.pop_back();
                            }
                        } else {
                            if (key > 0) {
                                if (key <= 125) {
                                    chatContent += static_cast<char>(key);
                                }
                            }

                        }

                        if (IsKeyPressed(KEY_ENTER)) {
                            if (!chatContent.empty() && chatContent[0] == '/') {
                                std::string input = chatContent.substr(1);
                                std::istringstream iss(input);
                                std::string command;
                                iss >> command;

                                if (command == "teleport" || command == "tp") {
                                    int x, y, z;
                                    if (!(iss >> x >> y >> z)) {
                                        std::cout << "Invalid teleport command." << std::endl;
                                    } else {
                                        camera.position = { static_cast<float>(x), static_cast<float>(y), static_cast<float>(z) };
                                    }
                                } else if (command == "setblock" || command == "sb") {
                                    int x, y, z, id;
                                    if (!(iss >> x >> y >> z >> id)) {
                                        std::cout << "Invalid setblock command.";
                                    } else {
                                        currentWorld.SetBlockId(x, y, z, id);
                                        if (!currentWorld.GetBlockId(x, y, z) == id) {
                                            std::cout << "Error placing block at " << x << y << z << std::endl;
                                        } else {
                                            std::cout << "Block successfully placed at " << x << y << z << std::endl;
                                        }
                                    }
                                } else {
                                    std::cout << "Unknown command." << std::endl;
                                    chatContent = "";
                                }
                            } else {
                                std::cout << "Message send:" << chatContent << std::endl;
                                chatContent = "";
                            }
                        }
                    }

                    if (IsKeyPressed(KEY_F3)) {
                        if (!f3enabled) {
                            f3enabled = true;
                        } else {
                            f3enabled = false;
                        }

                    }
                    if (IsKeyPressed(KEY_F1)) {
                        if (!HideHUD) {
                            HideHUD = true;
                        } else {
                            HideHUD = false;
                        }
                    }
                    if (IsKeyPressed(KEY_T)) {
                        std::cout << "Chat opened" << std::endl;
                        IsChatOpened = true;
                    }

                    if (IsMovementsEnabled) {            
                        if (IsKeyDown(KEY_W)) {
                            movement.x = player_speed;
                        }
                        if (IsKeyDown(KEY_S)) {
                            movement.x = -player_speed;
                        }
                        if (IsKeyDown(KEY_A)) {
                            movement.y = player_speed;
                        }
                        if (IsKeyDown(KEY_D)) {
                            movement.y = -player_speed;
                        }
                        if (IsKeyDown(KEY_SPACE)) {
                            movement.z = player_speed;
                        }
                        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                            movement.z = -player_speed;
                        }
                    }
                
                    UpdateCameraPro(&camera, movement, rotation, zoom);
                }

                BeginDrawing();
                ClearBackground(backgroundColor);
                BeginMode3D(camera);
                //--------------3D Drawing-----------------------
                DrawGrid(10, 1.0f);

                for (int chunkIndex = 0; chunkIndex < currentWorld.GetChunkCount(); ++chunkIndex) {
                    auto chunk = currentWorld.GetChunk(chunkIndex);
                    if (chunk) {
                        if (chunkIndex * 16 < 32) {
                            for (int x = 0; x < 16; ++x) {
                                for (int y = 0; y < 16; ++y) {
                                    for (int z = 0; z < 16; ++z) {
                                        int blockId = currentWorld.GetBlockId(x, y, z);
                                        if (blockId != 0) {
                                            DrawCubeTexture(texture, (Vector3){ x, y, z}, 1.0f, 1.0f, 1.0f, true, true, true, true, true, true, WHITE);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }

                if (cameraMode == CAMERA_THIRD_PERSON) {
                    DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
                    DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
                }

                EndMode3D();

                //-------------------2D Drawing--------------------

                if (f3enabled && !HideHUD) {
                    DrawFPS(10, 10);
                    DrawText("Nyx build pre-release 1.0.0", 15, f3textSpacing, f3textsize, RAYWHITE);
                    DrawText("Camera controls:", 15, f3textSpacing + f3lineSize, f3textsize, RAYWHITE);
                    DrawText("W, A, S, D, Space, Shift to move", 15, f3textSpacing + f3lineSize*2, f3textsize, RAYWHITE);
                    DrawText("Arrow keys or mouse to look around", 15, f3textSpacing + f3lineSize*3, f3textsize, RAYWHITE);
                    DrawText("T to open chat", 15, f3textSpacing + f3lineSize*4, f3textsize, RAYWHITE);
                    DrawText("Zoom keys: num-plus, num-minus or mouse scroll", 15, f3textSpacing + f3lineSize*5, f3textsize, RAYWHITE);

                    DrawText("Current camera status:", 610, f3textSpacing, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Mode: %s", (cameraMode == CAMERA_FREE) ? "FREE" :
                        (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
                        (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
                        (cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"), 610, f3textSpacing + f3lineSize, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Projection: %s", (camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
                        (camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"), 610, f3textSpacing + f3lineSize*2, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", 
                        camera.position.x, camera.position.y, camera.position.z), 610, f3textSpacing + f3lineSize*3, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", 
                        camera.target.x, camera.target.y, camera.target.z), 610, f3textSpacing + f3lineSize*4, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", 
                        camera.up.x, camera.up.y, camera.up.z), 610, f3textSpacing + f3lineSize*5, f3textsize, RAYWHITE);
                }

                if (IsChatOpened) {
                    DrawText(chatContent.c_str(), 15, GetRenderHeight() - 15 - f3textsize, f3textsize, RAYWHITE);
                }

                if (IsGamePaused) {
                    DrawText("Game Paused", GetRenderWidth() / 2 - 100, GetRenderHeight() / 2 -  25, 50, RAYWHITE);
                }
                EndDrawing();
                break;
            }

            case OPTIONS: {
                float totalButtonWidth = 2 * buttonWidth + buttonSpacing;
                float buttonX1 = (GetRenderWidth() - totalButtonWidth) / 2;
                float buttonX2 = buttonX1 + buttonWidth + buttonSpacing;
                float buttonY = GetRenderHeight() - buttonHeight - 20;

                if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    Vector2 mousePointOptions = { static_cast<float>(GetMouseX()), static_cast<float>(GetMouseY()) };

                    if (CheckMousePosition(buttonX1, buttonY, buttonWidth, buttonHeight)) {
                        std::cout << "Clicked Quit Game" << std::endl;
                        UnloadTexture(texture);
                        UnloadTexture(background);
                        CloseWindow();
                    } else if (CheckMousePosition(buttonX2, buttonY, buttonWidth, buttonHeight)) {
                        std::cout << "Clicked Back to menu" << std::endl;
                        currentScreen = MENU;
                    }
                }

                BeginDrawing();
                ClearBackground(backgroundColor);
                DrawTexture(background, 0, 0, WHITE);
                DrawButton(buttonX1, buttonY, buttonWidth, buttonHeight, 5, WHITE, backgroundColor, WHITE, "Quit Game");
                DrawButton(buttonX2, buttonY, buttonWidth, buttonHeight, 5, WHITE, backgroundColor, WHITE, "Back to menu");
                EndDrawing();
                break;
            }
        }
    }

    //UnloadModel(cubeModel);
    UnloadTexture(texture);
    UnloadTexture(background);
    CloseWindow();
}
