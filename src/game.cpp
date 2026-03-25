#include "game.hpp"

enum GameScreen { MENU, GAME, OPTIONS };

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

    GameModes gamemode;
    gamemode = CREATIVE;

    World currentWorld("Default World", {0.0f, 0.0f, 0.0f});
    int range = 1;

    for (int cx = -range; cx <= range; ++cx) {
        for (int cy = -range; cy <= range; ++cy) {
            for (int cz = -range; cz <= range; ++cz) {
                auto chunk = std::make_shared<Chunk>(cx, cy, cz);
                currentWorld.AddChunk(chunk);
                for (int i = 0; i < CHUNK_SIZE; ++i)
                    for (int j = 0; j < CHUNK_SIZE; ++j)
                        for (int k = 0; k < CHUNK_SIZE; ++k)
                            chunk->SetBlockId(i, j, k, 0);
            }
        }
    }

    currentWorld.SetBlockId(0, 0, 0, 1);
    currentWorld.SetBlockId(1, 0, 0, 1);
    currentWorld.SetBlockId(0, 0, 1, 1);
    currentWorld.SetBlockId(1, 0, 1, 1);
    currentWorld.SetBlockId(1, 0, 1, 1);
    currentWorld.SetBlockId(-1, 0, 0, 1);
    currentWorld.SetBlockId(0, 0, -1, 1);
    currentWorld.SetBlockId(-1, 0, -1, 1);
    currentWorld.SetBlockId(-1, 0, 1, 1);
    currentWorld.SetBlockId(1, 0, -1, 1);


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
    Color fpsColor = GREEN;
    int currentFPS = 0;

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

                Vector3 camPos = camera.position;

                if (IsKeyPressed(KEY_ESCAPE)) {
                    if (IsChatOpened) {
                        IsChatOpened = false;
                        chatContent = "";
                        IsMovementsEnabled = true;
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
                                        if (currentWorld.GetBlockId(x,y,z) != id) {
                                            std::cout << "Error placing block at " << x << y << z << std::endl;
                                        } else {
                                            std::cout << "Block successfully placed at " << x << y << z << std::endl;
                                        }
                                    }
                                } else if (command == "fill") {
                                    int x1, y1, z1, x2, y2, z2, id;
                                    std::string mode = "replace";
                                    int placed = 0;

                                    if (!(iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> id)) {
                                        std::cout << "Invalid fill command. Usage: /fill x1 y1 z1 x2 y2 z2 id [replace|keep]" << std::endl;
                                    } else {
                                        int minX = std::min(x1, x2);
                                        int maxX = std::max(x1, x2);
                                        int minY = std::min(y1, y2);
                                        int maxY = std::max(y1, y2);
                                        int minZ = std::min(z1, z2);
                                        int maxZ = std::max(z1, z2);

                                        const int MAX_VOLUME = 20000;
                                        long long vol = (long long)(maxX - minX + 1) * (maxY - minY + 1) * (maxZ - minZ + 1);
                                        if (vol <= 0 || vol > MAX_VOLUME) {
                                            std::cout << "Fill area too large or invalid (" << vol << " blocks). Aborted." << std::endl;
                                        } else {
                                            for (int xi = minX; xi <= maxX; ++xi) {
                                                for (int yi = minY; yi <= maxY; ++yi) {
                                                    for (int zi = minZ; zi <= maxZ; ++zi) {
                                                        int currentId = currentWorld.GetBlockId(xi, yi, zi);
                                                        bool doPlace = false;
                                                        if (mode == "keep") {
                                                            if (currentId == 0) doPlace = true;
                                                        } else { // replace (default)
                                                            doPlace = true;
                                                        }

                                                        if (doPlace) {
                                                            currentWorld.SetBlockId(xi, yi, zi, id);
                                                            if (currentWorld.GetBlockId(xi, yi, zi) == id) {
                                                                ++placed;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    std::cout << "Filled area with id " << id << ". Blocks placed: " << placed << std::endl;
                                
                                } else {
                                    std::cout << "Unknown command." << std::endl;
                                }
                            } else {
                                std::cout << "Message send:" << chatContent << std::endl;
                            }
                            chatContent = "";
                            IsChatOpened = false;
                            IsMovementsEnabled = true;
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
                        IsMovementsEnabled = false;
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
                
                    //if (IsKeyPressed(MOUSE_LEFT_BUTTON)) {
                    //    Vector2 screenCenter = { GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
                    //    Ray ray = GetMouseRay(screenCenter, camera);

                    //}

                    UpdateCameraPro(&camera, movement, rotation, zoom);
                }

                BeginDrawing();
                ClearBackground(backgroundColor);
                BeginMode3D(camera);
                //--------------3D Drawing-----------------------
                DrawGrid(10, 1.0f);

                // get camera position in int
                int wx = (int)floorf(camPos.x);
                int wy = (int)floorf(camPos.y);
                int wz = (int)floorf(camPos.z);

                auto [camCx, camLx] = World::WorldToChunkAndLocal(wx);
                auto [camCy, camLy] = World::WorldToChunkAndLocal(wy);
                auto [camCz, camLz] = World::WorldToChunkAndLocal(wz);

                for (int dx = -renderDistance; dx <= renderDistance; ++dx) {
                    for (int dy = -renderDistance; dy <= renderDistance; ++dy) {
                        for (int dz = -renderDistance; dz <= renderDistance; ++dz) {
                            int cx = camCx + dx;
                            int cy = camCy + dy;
                            int cz = camCz + dz;
                            auto chunk = currentWorld.GetChunkAt(cx, cy, cz);
                            if (!chunk) continue;

                            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                                for (int ly = 0; ly < CHUNK_SIZE; ++ly) {
                                    for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                                        int worldX = cx * CHUNK_SIZE + lx;
                                        int worldY = cy * CHUNK_SIZE + ly;
                                        int worldZ = cz * CHUNK_SIZE + lz;

                                        int blockId = currentWorld.GetBlockId(worldX, worldY, worldZ);
                                        if (blockId == 0) continue;

                                        bool drawFront  = currentWorld.IsBlockTransparent(worldX,     worldY,     worldZ+1);
                                        bool drawBack   = currentWorld.IsBlockTransparent(worldX,     worldY,     worldZ-1);
                                        bool drawTop    = currentWorld.IsBlockTransparent(worldX,     worldY+1,   worldZ);
                                        bool drawBottom = currentWorld.IsBlockTransparent(worldX,     worldY-1,   worldZ);
                                        bool drawRight  = currentWorld.IsBlockTransparent(worldX+1,   worldY,     worldZ);
                                        bool drawLeft   = currentWorld.IsBlockTransparent(worldX-1,   worldY,     worldZ);

                                        if (!(drawFront||drawBack||drawTop||drawBottom||drawRight||drawLeft)) continue;

                                        Vector3 pos = { (float)worldX, (float)worldY, (float)worldZ };
                                        DrawCubeTexture(texture, pos, 1.0f, 1.0f, 1.0f,
                                            drawFront, drawBack, drawTop, drawBottom, drawRight, drawLeft, WHITE);
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
                    currentFPS = GetFPS();
                    if (currentFPS < 20) {
                        fpsColor = ORANGE;
                    } else if (currentFPS < 10) {
                        fpsColor = RED;
                    } else {
                        fpsColor = GREEN;
                    }
                    DrawText(TextFormat("FPS: %i (Target: 60)", GetFPS()), 15, f3textSpacing, f3textsize, fpsColor);
                    DrawText("Nyx build pre-release 1.0.0", 15, f3textSpacing + f3lineSize, f3textsize, RAYWHITE);
                    DrawText("Camera controls:", 15, f3textSpacing + f3lineSize*2, f3textsize, RAYWHITE);
                    DrawText("W, A, S, D, Space, Shift to move", 15, f3textSpacing + f3lineSize*3, f3textsize, RAYWHITE);
                    DrawText("Arrow keys or mouse to look around", 15, f3textSpacing + f3lineSize*4, f3textsize, RAYWHITE);
                    DrawText("T to open chat", 15, f3textSpacing + f3lineSize*5, f3textsize, RAYWHITE);
                    DrawText("Zoom keys: num-plus, num-minus or mouse scroll", 15, f3textSpacing + f3lineSize*6, f3textsize, RAYWHITE);

                    DrawText("Current camera status:", 15, f3textSpacing + f3lineSize*7, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Mode: %s", (cameraMode == CAMERA_FREE) ? "FREE" :
                        (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
                        (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
                        (cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"), 15, f3textSpacing + f3lineSize*8, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Projection: %s", (camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
                        (camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"), 15, f3textSpacing + f3lineSize*9, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Position: (%06.3f, %06.3f, %06.3f)", 
                        camera.position.x, camera.position.y, camera.position.z), 15, f3textSpacing + f3lineSize*10, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Target: (%06.3f, %06.3f, %06.3f)", 
                        camera.target.x, camera.target.y, camera.target.z), 15, f3textSpacing + f3lineSize*11, f3textsize, RAYWHITE);
                    DrawText(TextFormat("- Up: (%06.3f, %06.3f, %06.3f)", 
                        camera.up.x, camera.up.y, camera.up.z), 15, f3textSpacing + f3lineSize*12, f3textsize, RAYWHITE);
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