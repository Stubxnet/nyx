#include "game.hpp"

void run(const Config& config) {
    const char* title = config.windowTitle.c_str();
    InitWindow(config.windowWidth, config.windowHeight, title);

    GameScreen currentScreen = MENU;

    int renderDistance = 2;

    bool checkblock = true;

    SetExitKey(KEY_NULL);

    int targetFPS = 60;

    SetTargetFPS(targetFPS);

    //////////////////////////////////////////////////////////////////////////
    /////////////////            TEXTURES LOADING            /////////////////
    //////////////////////////////////////////////////////////////////////////

    Texture2D background = LoadTexture(genPath(config.gameDirectory, 
                  "assets/textures/background/background1920x1080p.png").c_str());
    
    Image atlasImage = LoadImage(genPath(config.gameDirectory, "assets/textures/atlas.png").c_str());
    Texture2D atlas = LoadTextureFromImage(atlasImage);

    UnloadImage(atlasImage);

    //////////////////////////////////////////////////////////////////////////
    /////////////////      WORLD CLASS INITIALIZATION        /////////////////
    //////////////////////////////////////////////////////////////////////////

    GameRules gamerules;

    GameModes currentGamemode;
    currentGamemode = CREATIVE;

    World currentWorld("Default World", {0.0f, 6.0f, 2.0f});
    int range = renderDistance;

    for (int cx = -range; cx <= range; ++cx) {    
        for (int cy = -range; cy <= range; ++cy) {        
            for (int cz = -range; cz <= range; ++cz) {            
                auto chunk = std::make_shared<Chunk>(cx, cy, cz);            
                currentWorld.AddChunk(chunk);        
            }    
        }
    }
    
    SetAtlasTexture(atlas);
    SetAtlasParams(TILE, ATLAS_COLS, ATLAS_ROWS);

    for (size_t i = 0; i < currentWorld.GetChunkCount(); ++i) {
        auto ch = currentWorld.GetChunk(i);
        if (!ch) continue;
        int32_t cx = ch->GetChunkX();
        int32_t cy = ch->GetChunkY();
        int32_t cz = ch->GetChunkZ();

        currentWorld.MarkChunkAsDirty(cx, cy, cz);
    }

    currentWorld.SetChunkModifiedCallback([&](int32_t cx,int32_t cy,int32_t cz){
        currentWorld.MarkChunkAsDirty(cx, cy, cz);
    });
    
    currentWorld.FillBlocks(8, 0, 8, -8, 0, -8, BlockFillActions::SET);
    currentWorld.SetBlock(0, 1, 0, 2);
    currentWorld.SetBlock(1, 2, 0, 3);
    currentWorld.SetBlock(2, 3, 0, 4);
    currentWorld.SetBlock(3, 4, 0, 5);
    currentWorld.SetBlock(4, 5, 0, 6);
    currentWorld.SetBlock(5, 6, 0, 7);
    currentWorld.SetBlock(6, 7, 0, 8);
    currentWorld.SetBlock(7, 8, 0, 9);
    currentWorld.FillBlocks(7, 8, 1, -7, 8, 1, BlockFillActions::SET, 23);


    //////////////////////////////////////////////////////////////////////////
    //////////////////                 CAMERA               //////////////////
    //////////////////////////////////////////////////////////////////////////

    Vector3 currentSpawnPoint = currentWorld.GetSpawnPoint();
    RenderState renderState = { 0 };
    Body body = { 0 };

    Camera3D camera = { 0 };
    camera.position = currentSpawnPoint;

    renderState.currentCameraPosition = camera.position;
    renderState.previousCameraPosition = camera.position;
    camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
    camera.fovy = 60.0f;
    camera.projection = CAMERA_PERSPECTIVE;

    int cameraMode = CAMERA_FIRST_PERSON;

    float player_speed = 0.1f;                // TODO: Define a class Entity and create an instance for the player
    Vector3 rotation = {0.0f, 0.0f, 0.0f};
    Vector3 movement = {0.0f, 0.0f, 0.0f};

    bool collided = false;

    float zoom = GetMouseWheelMove() * 0.5f;

    static Vector2 sensitivity = { 0.001f, 0.001f };
    static float globalTime = 0.0f;
    static float tickAccumulator = 0.0f;

    float accumulator = 0.0f;

    bool isCreativeFlyEnabled = true;
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

    //----------- HUD-related -----------------

    // F3
    bool f3enabled = true;
    int f3textsize = GetRenderHeight() / 40;
    int f3textSpacing = f3textsize / 2;
    int f3lineSize = f3textSpacing + f3textsize;
    Color fpsColor = GREEN;
    Color f3color = RAYWHITE;
    int currentFPS = 0;
    bool drawBoundingBoxes = false;
    bool drawChunksGrid = false;

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
                MenuAction action = DrawAndHandleMenu(screenheight, screenwidth, background, backgroundColor, backgroundColor);
                if (action == MenuAction::PLAY) currentScreen = GAME;
                else if (action == MenuAction::OPTIONS) currentScreen = OPTIONS;
                else if (action == MenuAction::QUIT) {
                    UnloadTexture(atlas);
                    UnloadTexture(background);
                    CloseWindow();
                }
                break;
            }

            case GAME: {
                int chunksCount = static_cast<int>(currentWorld.GetChunkCount());

                int key = GetCharPressed();
                renderState.previousCameraPosition = camera.position;

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
                    //--------------Data update----------------
                    float frame_dt = GetFrameTime();    
                    if (frame_dt > 0.25f) frame_dt = 0.25f;

                    Vector2 mouseDelta = GetMouseDelta();    
                    zoom = GetMouseWheelMove() * 0.5f;

                    rotation.x -= mouseDelta.x * sensitivity.x; // yaw
                    rotation.y -= mouseDelta.y * sensitivity.y; // pitch

                    //-------------Update dirty chunks-----------
                    for (size_t i = 0; i < currentWorld.GetChunkCount(); ++i) {
                        auto ch = currentWorld.GetChunk(i);
                        if (!ch) continue;
                        int32_t cx = ch->GetChunkX();
                        int32_t cy = ch->GetChunkY();
                        int32_t cz = ch->GetChunkZ();

                        if (!currentWorld.IsChunkDirty(cx, cy, cz)) continue;

                        if (currentWorld.IsChunkLoaded(cx, cy, cz)) {
                            currentWorld.UnloadChunk(cx, cy, cz);
                            currentWorld.UnmarkChunkAsLoaded(cx, cy, cz);
                        }

                        Model m = BuildModelForChunk(ch, &currentWorld);
                        if (m.meshCount > 0) {
                            currentWorld.UpdateChunkModel(cx, cy, cz, m);
                            if (atlas.id) currentWorld.SetChunkMaterialTexture(cx, cy, cz, atlas);
                            currentWorld.MarkChunkAsLoaded(cx, cy, cz);
                        } else {
                            Model empty = Model{0};
                            currentWorld.UpdateChunkModel(cx, cy, cz, empty);
                            currentWorld.UnmarkChunkAsLoaded(cx, cy, cz);
                        }

                        currentWorld.UnmarkChunkAsDirty(cx, cy, cz);
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
                                auto cameraPtr = std::make_shared<Camera3D>(camera);
                                auto worldPtr  = std::make_shared<World>(currentWorld);
                                CommandContext ctx{ cameraPtr, worldPtr, &renderDistance, &currentGamemode, &isCreativeFlyEnabled };
                                HandleCommand(input, ctx);
                            } else {
                                std::cout << "Message send:" << chatContent << std::endl;
                            }
                            chatContent.clear();
                            IsChatOpened = false;
                            IsMovementsEnabled = true;
                        }
                    }

                    if (IsKeyPressed(KEY_F3)) {
                        if (IsKeyPressed(KEY_B)) {
                            drawBoundingBoxes = true;
                        } else if (IsKeyPressed(KEY_G)) {
                            drawChunksGrid = true;
                        } else if (!f3enabled) {
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

                    if (IsKeyPressed(KEY_F5)) {
                        if (cameraMode != CAMERA_THIRD_PERSON) {
                            cameraMode = CAMERA_THIRD_PERSON;
                        } else {
                            cameraMode = CAMERA_FIRST_PERSON;
                        }
                    }

                    if (IsKeyPressed(KEY_T)) {
                        std::cout << "Chat opened" << std::endl;
                        IsMovementsEnabled = false;
                        IsChatOpened = true;
                    }

                    //------------Tick update------------------------
                    UpdatePlayerMovementTick(
                        camera,
                        body,
                        renderState,
                        currentWorld,
                        currentGamemode,
                        isCreativeFlyEnabled,
                        movement,
                        rotation,
                        accumulator,
                        tickAccumulator,
                        frame_dt,
                        mouseDelta,
                        zoom,
                        IsMovementsEnabled,
                        IsMouseEnabled
                    );
                    
//                    UpdateCameraPro(&camera, movement, rotation, zoom);
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

                for (int ox = -renderDistance; ox <= renderDistance; ++ox) {
                    for (int oy = -renderDistance; oy <= renderDistance; ++oy) {
                        for (int oz = -renderDistance; oz <= renderDistance; ++oz) {
                            int cx = camCx + ox;
                            int cy = camCy + oy;
                            int cz = camCz + oz;

                            if (!currentWorld.IsChunkLoaded(cx, cy, cz)) continue;

                            auto chunk = currentWorld.GetChunkAt(cx, cy, cz);
                            if (!chunk) continue;

                            Model model = chunk->GetModel();
                            if (chunk->IsModelEmpty()) continue;

                            float worldX = (float)(cx * CHUNK_SIZE);
                            float worldY = (float)(cy * CHUNK_SIZE);
                            float worldZ = (float)(cz * CHUNK_SIZE);

                            // TODO: frustum culling

                            DrawModel(model, (Vector3){ worldX, worldY, worldZ }, 1.0f, WHITE);

                            currentWorld.Rendered(cx, cy, cz);
                        }
                    }
                }

                if (drawBoundingBoxes && camera.projection == CAMERA_THIRD_PERSON) {   
                    BoundingBox playerBox = CreatePlayerHitbox(camera);
                    DrawBoundingBox(playerBox, LIME);
                }

                EndMode3D();

                //-------------------2D Drawing--------------------

                //--------------------Cursor-----------------------
                int centerx = GetScreenWidth() / 2;
                int centery = GetScreenHeight() / 2;
                DrawLine(centerx - 10, centery, centerx - 3, centery, WHITE);
                DrawLine(centerx + 3,  centery, centerx + 10, centery, WHITE);
                DrawLine(centerx, centery - 10, centerx, centery - 3, WHITE);
                DrawLine(centerx, centery + 3,  centerx, centery + 10, WHITE);


                //----------------------F3------------------------
                if (f3enabled && !HideHUD) {
                    currentFPS = GetFPS();
                    if (currentFPS < 30) fpsColor = ORANGE;
                    else if (currentFPS < 10) fpsColor = RED;
                    else fpsColor = GREEN;

                    DrawText(TextFormat("FPS: %i (Target: %i)", GetFPS(), targetFPS), 15, f3textSpacing, f3textsize, fpsColor);
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
                    DrawText(TextFormat("Render distance: %d", renderDistance), 15, f3textSpacing + f3lineSize*15, f3textsize, f3color);
                    DrawText(TextFormat("Gamemode: %s",
                        (currentGamemode == SURVIVAL) ? "SURVIVAL" :
                        (currentGamemode == CREATIVE) ? "CREATIVE" :
                        (currentGamemode == SPECTATOR) ? "SPECTATOR" : 
                        (currentGamemode == BUILDER) ? "BUILDER" : "UNKNOWN"
                    ), 15, f3textSpacing + f3lineSize*16, f3textsize, f3color);
                }

                if (IsChatOpened && !HideHUD) {
                    DrawChat(screenheight, screenwidth, chatTextsize, chatContent);
                }

                if (IsGamePaused) {
                    DrawPauseScreen(screenheight, screenwidth);
                } 
                EndDrawing();
                break;
            }

            case OPTIONS: {    
                OptionsAction action = DrawAndHandleOptions(screenwidth, screenheight, background, backgroundColor, backgroundColor);    
                if (action == OptionsAction::QUIT) {        
                    UnloadTexture(atlas);        
                    UnloadTexture(background);        
                    CloseWindow();    
                } else if (action == OptionsAction::BACK) {        
                    currentScreen = MENU;    
                }    
                break;
            }        
        }
    }

    UnloadTexture(atlas);
    UnloadTexture(background);
    CloseWindow();
}