#include "game.hpp"

enum GameScreen { MENU, GAME, OPTIONS };

void Game::init() {
    // TODO: use this initialization function
}

void Game::run(const Config& config) {
    const char* title = config.windowTitle.c_str();
    InitWindow(config.windowWidth, config.windowHeight, title);

    GameScreen currentScreen = MENU;

    int renderDistance = 2;

    bool checkblock = true;

    SetExitKey(KEY_NULL);

    SetTargetFPS(60);

    //////////////////////////////////////////////////////////////////////////
    /////////////////      WORLD CLASS INITIALIZATION        /////////////////
    //////////////////////////////////////////////////////////////////////////

    GameRules gamerules;

    GameModes currentGamemode;
    currentGamemode = CREATIVE;

    World currentWorld("Default World", {0.0f, 0.0f, 0.0f});

    int range = renderDistance;

    for (int cx = -range; cx <= range; ++cx) {
        for (int cy = -range; cy <= range; ++cy) {
            for (int cz = -range; cz <= range; ++cz) {
                auto chunk = std::make_shared<Chunk>(cx, cy, cz);
                currentWorld.AddChunk(chunk);
            }
        }
    }

    currentWorld.FillBlocks(-3, -3, -3, 3, 3, 3, BlockFillActions::SET);
    currentWorld.FillBlocks(-2, -2, -2, 2, 2, 2, BlockFillActions::SET, 0);
    currentWorld.SetBlock(0, 0, 0, 5);

    //////////////////////////////////////////////////////////////////////////
    /////////////////            TEXTURES LOADING            /////////////////
    //////////////////////////////////////////////////////////////////////////

    Texture2D background = LoadTexture(genPath(config.gameDirectory, 
                  "assets/textures/background/background1920x1080p.png").c_str());
    
    Image atlasImage = LoadImage(genPath(config.gameDirectory, "assets/textures/atlas.png").c_str());
    Texture2D atlas = LoadTextureFromImage(atlasImage);

    UnloadImage(atlasImage);

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

    float player_speed = 0.1f;                // TODO: Define a class Entity and create an instance for the player
    Vector3 rotation = {0.0f, 0.0f, 0.0f};
    Vector3 movement = {0.0f, 0.0f, 0.0f};
    Vector3 previousCameraPosition = camera.position;
    BoundingBox playerBB;
    bool collided = false;

    float zoom = GetMouseWheelMove() * 0.5f;

    //-------------------------------------
    // Vars

    bool IsGamePaused = false;
    bool IsMouseEnabled = false;
    bool IsMovementsEnabled = true;

    int screenheight = GetRenderHeight();
    int screenwidth = GetRenderWidth();

    // MENU screen
    float buttonWidth = 200;
    float buttonHeight = 50;
    float buttonSpacing = 20;

    //----------- HUD-related functions-------------

    // F3
    bool f3enabled = true;
    int f3textsize = GetRenderHeight() / 40;
    int f3textSpacing = f3textsize / 2;
    int f3lineSize = f3textSpacing + f3textsize;
    Color fpsColor = GREEN;
    Color f3color = RAYWHITE;
    int currentFPS = 0;

    // F1
    int HideHUD = false;

    // Chat
    int IsChatOpened = false;
    std::string chatContent;
    int chatTextsize = GetRenderHeight() / 40;

    while (!WindowShouldClose()) {
        if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyDown(KEY_C)) {
            std::cout << "Pressed Ctrl+C. Exiting." << std::endl;
            break;
        }
        
        switch (currentScreen) {
            case MENU: {
                float buttonXPlay = screenwidth / 2 - buttonWidth / 2;
                float buttonYPlay = screenheight / 2 - buttonHeight - 20;
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
                        UnloadTexture(atlas);
                        UnloadTexture(background);
                        CloseWindow();
                    }
                }

                DrawMenu(screenheight, screenwidth, buttonXPlay, buttonYPlay, buttonYOptions, buttonYQuit, buttonWidth, buttonHeight, backgroundColor, background, backgroundColor);
                break;
            }

            case GAME: {
                int chunksCount = static_cast<int>(currentWorld.GetChunkCount());
                int solidBlocksCount = 0;

                int key = GetCharPressed();
                previousCameraPosition = camera.position;

                if (IsKeyPressed(KEY_ESCAPE)) {
                    if (IsChatOpened) {
                        IsChatOpened = false;
                        chatContent = "";
                        IsMovementsEnabled = true;
                        std::cout << "Chat closed" << std::endl;
                    } else {
                        if (IsGamePaused) {
                            IsGamePaused = false;
                            std::cout << "Game restarted." << std::endl;
                            DisableCursor();
                        } else if (!IsGamePaused) {
                            IsGamePaused = true;
                            std::cout << "Game paused." << std::endl;
                            ShowCursor();
                        }
                    }
                }

                if (!IsGamePaused) {
                    rotation.x = GetMouseDelta().x * 0.1f;
                    rotation.y = GetMouseDelta().y * 0.1f;

                    if (currentGamemode == CREATIVE || currentGamemode == SPECTATOR) {
                        movement = {0.0f, 0.0f, 0.0f};
                    } else {
                        movement.x = movement.x * 0.98;
                        movement.y = movement.y * 0.98;
                        movement.z = movement.z * 0.98;
                    }
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
                                    float x, y, z;
                                    if (!(iss >> x >> y >> z)) {
                                        std::cout << "Invalid teleport command." << std::endl;
                                    } else {
                                        camera.position = { x, y, z };
                                    }
                                } else if (command == "rotation" || command == "rt") {
                                    float x, y, z;
                                    if(!(iss >> x >> y >> z)) {
                                        std::cout << "Invalid rotation command." << std::endl;
                                    } else {
                                        camera.target = { x, y, z };
                                    }
                                } else if (command == "fov") {
                                    float fov;
                                    if (!(iss >> fov)) {
                                        std::cout << "Invalid fov command." << std::endl;
                                    } else {
                                        camera.fovy = fov;
                                    }
                                } else if (command == "renderdistance" || command == "rd") {
                                    int rd;
                                    if (!(iss >> rd)) {
                                        std::cout << "Invalid renderdistance command." << std::endl;
                                    } else {
                                        renderDistance = rd;
                                    }
                                } else if (command == "setblock" || command == "sb") {
                                    int x, y, z, id;
                                    if (!(iss >> x >> y >> z >> id)) {
                                        std::cout << "Invalid setblock command." << std::endl;
                                    } else {
                                        currentWorld.SetBlock(x, y, z, id);
                                        if (currentWorld.GetBlockId(x,y,z) != id) {
                                            std::cout << "Error placing block at " << x << y << z << std::endl;
                                        } else {
                                            std::cout << "Block successfully placed at " << x << y << z << std::endl;
                                        }
                                    }
                                } else if (command == "fill") {
                                    int x1, y1, z1, x2, y2, z2, id;
                                    std::string modeStr;
                                    int placed = 0;

                                    if (!(iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> id)) {
                                        std::cout << "Invalid fill command. Usage: /fill x1 y1 z1 x2 y2 z2 id [replace|keep|break|outline|set]" << std::endl;
                                    } else {
                                        if (iss >> modeStr) {
                                            std::transform(modeStr.begin(), modeStr.end(), modeStr.begin(), ::tolower);
                                        } else {
                                            modeStr = "set";
                                        }

                                        auto ParseFillAction = [](const std::string &s) -> BlockFillActions {
                                            if (s == "set") return BlockFillActions::SET;
                                            if (s == "replace") return BlockFillActions::REPLACE;
                                            if (s == "keep") return BlockFillActions::KEEP;
                                            if (s == "break") return BlockFillActions::BREAK;
                                            if (s == "outline") return BlockFillActions::OUTLINE;
                                            return BlockFillActions::SET;
                                        };

                                        BlockFillActions action = ParseFillAction(modeStr);

                                        placed = currentWorld.FillBlocks(x1, y1, z1, x2, y2, z2, action, id);
                                        if (placed > 0) {
                                            std::cout << "Filled area with id " << id << " (mode: " << modeStr << "). Blocks placed: " << placed << std::endl;
                                        } else {
                                            std::cout << "Failed to set blocks in the specified area." << std::endl;
                                        }
                                    }
                                } else if (command == "gamemode" || command == "gm") {
                                    std::string stringMode;
                                    int intMode;

                                    if (iss >> intMode) {
                                        switch (intMode) {
                                            case 0:
                                                currentGamemode = SURVIVAL;
                                                break;
                                            case 1:
                                                currentGamemode = CREATIVE;
                                                break;
                                            case 2:
                                                currentGamemode = SPECTATOR;
                                                break;
                                            default: 
                                                currentGamemode = SURVIVAL;
                                                break;
                                        }
                                    } else if (iss >> stringMode) {
                                        if (stringMode == "survival") currentGamemode = SURVIVAL;
                                        else if (stringMode == "creative") currentGamemode = CREATIVE;
                                        else if (stringMode == "spectator") currentGamemode = SPECTATOR;
                                        else currentGamemode = SURVIVAL;

                                    } else {
                                        std::cout << "Invalid gamemode command." << std::endl;
                                    }
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
                        if (IsKeyDown(KEY_D)) {
                            movement.y = player_speed;
                        }
                        if (IsKeyDown(KEY_A)) {
                            movement.y = -player_speed;
                        }
                        if (IsKeyDown(KEY_SPACE)) {
                            movement.z = player_speed;
                        }
                        if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
                            movement.z = -player_speed;
                        }
                    }
                
                    //------------------Collision detection----------------------
                    playerBB = CameraToPlayerBB(camera.position);
                    int ix0 = (int)floor(playerBB.min.x);
                    int ix1 = (int)ceil(playerBB.max.x) - 1;
                    int iy0 = (int)floor(playerBB.min.y);
                    int iy1 = (int)ceil(playerBB.max.y) - 1;
                    int iz0 = (int)floor(playerBB.min.z);
                    int iz1 = (int)ceil(playerBB.max.z) - 1;

                    for(int x = ix0; x <= ix1; ++x){
                        for(int y = iy0; y <= iy1; ++y){
                            for(int z = iz0; z <= iz1; ++z){
                                if (currentWorld.GetBlockId(x, y, z) != 0) collided = true;
                                if (checkblock = true) {
                                    std::cout << "Checked block at:" << x << " " << y << " " << z << std::endl;
                                    checkblock = false;
                                }
                            }
                        }
                    }

                    if (collided){
                        camera.position = previousCameraPosition;
                    } else {
                        previousCameraPosition = camera.position;
                    }

                    collided = false;

                    UpdateCameraPro(&camera, movement, rotation, zoom);
                }
                //------------------Drawing----------------------
                BeginDrawing();
                ClearBackground(backgroundColor);
                BeginMode3D(camera);
                //--------------3D Drawing-----------------------

                // get camera position in int
                int wx = (int)floorf(camera.position.x);
                int wy = (int)floorf(camera.position.y);
                int wz = (int)floorf(camera.position.z);

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
                            if (!chunk) {
                                auto newChunk = std::make_shared<Chunk>(cx, cy, cz);
                                currentWorld.AddChunk(newChunk);
                                chunk = currentWorld.GetChunkAt(cx,cy, cz);
                            }

                            if (chunk->IsChunkEmpty()) continue; // skip empty chunks

                            for (int lx = 0; lx < CHUNK_SIZE; ++lx) {
                                for (int ly = 0; ly < CHUNK_SIZE; ++ly) {
                                    for (int lz = 0; lz < CHUNK_SIZE; ++lz) {
                                        int worldX = cx * CHUNK_SIZE + lx;
                                        int worldY = cy * CHUNK_SIZE + ly;
                                        int worldZ = cz * CHUNK_SIZE + lz;

                                        int blockId = currentWorld.GetBlockId(worldX, worldY, worldZ);
                                        if (blockId == 0) continue;

                                        solidBlocksCount++;

                                        bool drawFront  = currentWorld.IsBlockTransparent(worldX,     worldY,     worldZ+1);
                                        bool drawBack   = currentWorld.IsBlockTransparent(worldX,     worldY,     worldZ-1);
                                        bool drawTop    = currentWorld.IsBlockTransparent(worldX,     worldY+1,   worldZ);
                                        bool drawBottom = currentWorld.IsBlockTransparent(worldX,     worldY-1,   worldZ);
                                        bool drawRight  = currentWorld.IsBlockTransparent(worldX+1,   worldY,     worldZ);
                                        bool drawLeft   = currentWorld.IsBlockTransparent(worldX-1,   worldY,     worldZ);

                                        int tileX_for_block, tileY_for_block;
                                        std::tie(tileX_for_block, tileY_for_block) = AtlasCoordsForBlock(blockId);

                                        if (tileX_for_block < 0) tileX_for_block = 0;
                                        if (tileX_for_block >= ATLAS_COLS) tileX_for_block = ATLAS_COLS - 1;
                                        if (tileY_for_block < 0) tileY_for_block = 0;
                                        if (tileY_for_block >= ATLAS_ROWS) tileY_for_block = ATLAS_ROWS - 1;

                                        Rectangle srcBlock = {
                                            float(tileX_for_block * TILE),
                                            float(tileY_for_block * TILE),
                                            float(TILE), float(TILE)
                                        };

                                        Vector3 pos = { (float)worldX, (float)worldY, (float)worldZ };
                                        DrawCubeTexture(atlas, srcBlock, pos, 1.0f, 1.0f, 1.0f,
                                            drawFront, drawBack, drawTop, drawBottom, drawRight, drawLeft, WHITE);
                                    }
                                }
                            }
                        }
                    }
                }

                if (cameraMode == CAMERA_THIRD_PERSON) {
                    DrawCube(camera.target, 0.5f, 0.5f, 0.5f, RED);
                    DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, RAYWHITE);
                }

                EndMode3D();

                //-------------------2D Drawing--------------------

                if (f3enabled && !HideHUD) {
                    currentFPS = GetFPS();
                    if (currentFPS < 30) fpsColor = ORANGE;
                    else if (currentFPS < 10) fpsColor = RED;
                    else fpsColor = GREEN;

                    DrawText(TextFormat("FPS: %i (Target: 60)", GetFPS()), 15, f3textSpacing, f3textsize, fpsColor);
                    DrawText("Nyx build pre-release 1.0.0", 15, f3textSpacing + f3lineSize, f3textsize, f3color);
                    DrawText("Camera controls:", 15, f3textSpacing + f3lineSize*2, f3textsize, f3color);
                    DrawText("W, A, S, D, Space, Shift to move", 15, f3textSpacing + f3lineSize*3, f3textsize, f3color);
                    DrawText("Arrow keys or mouse to look around", 15, f3textSpacing + f3lineSize*4, f3textsize, f3color);
                    DrawText("T to open chat", 15, f3textSpacing + f3lineSize*5, f3textsize, f3color);
                    DrawText("Zoom keys: num-plus, num-minus or mouse scroll", 15, f3textSpacing + f3lineSize*6, f3textsize, f3color);

                    DrawText("Current camera status:", 15, f3textSpacing + f3lineSize*7, f3textsize, f3color);
                    DrawText(TextFormat("Camera mode: %s", (cameraMode == CAMERA_FREE) ? "FREE" :
                        (cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
                        (cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
                        (cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"), 15, f3textSpacing + f3lineSize*8, f3textsize, f3color);
                    DrawText(TextFormat("Projection: %s", (camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
                        (camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"), 15, f3textSpacing + f3lineSize*9, f3textsize, f3color);
                    DrawText(TextFormat("Position: (%06.3f, %06.3f, %06.3f)", 
                        camera.position.x, camera.position.y, camera.position.z), 15, f3textSpacing + f3lineSize*10, f3textsize, f3color);
                    DrawText(TextFormat("Target: (%06.3f, %06.3f, %06.3f)", 
                        camera.target.x, camera.target.y, camera.target.z), 15, f3textSpacing + f3lineSize*11, f3textsize, f3color);
                    DrawText(TextFormat("Up: (%06.3f, %06.3f, %06.3f)", 
                        camera.up.x, camera.up.y, camera.up.z), 15, f3textSpacing + f3lineSize*12, f3textsize, f3color);
                    DrawText(TextFormat("Chunks on world: %d", chunksCount), 15, f3textSpacing + f3lineSize*13, f3textsize, f3color);
                    DrawText(TextFormat("Blocks on world (including air): %d", chunksCount*16), 15, f3textSpacing + f3lineSize*14, f3textsize, f3color);
                    DrawText(TextFormat("Blocks on world (excluding air): %d", solidBlocksCount), 15, f3textSpacing + f3lineSize*15, f3textsize, f3color);
                    DrawText(TextFormat("Gamemode: %s",
                        (currentGamemode == SURVIVAL) ? "SURVIVAL" :
                        (currentGamemode == CREATIVE) ? "CREATIVE" :
                        (currentGamemode == SPECTATOR) ? "SPECTATOR" : "UNKNOWN"
                    ), 15, f3textSpacing + f3lineSize*16, f3textsize, f3color);
                }

                if (IsChatOpened) {
                    DrawChat(screenheight, screenwidth, chatTextsize, chatContent);
                }

                if (IsGamePaused) {
                    DrawPauseScreen(screenheight, screenwidth);
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
                        UnloadTexture(atlas);
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
    UnloadTexture(atlas);
    UnloadTexture(background);
    CloseWindow();
}