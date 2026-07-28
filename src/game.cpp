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
    
    //-------------------Block defaults loading--------------------
    const fs::path gameDir = fs::path(config.gameDirectory);
    const fs::path texturesDir = gameDir / "assets/blocks/textures";
    const fs::path savesDir    = gameDir / "assets/blocks/saves";
    const fs::path atlasPngPath = savesDir / "atlas.png";
    const fs::path atlasUvPath  = savesDir / "atlas_uv.json";

    BlocksDefaults blocksDefaults;
    auto loaded = loadBlockReferences(genPath(config.gameDirectory, "assets/blocks/references/"), texturesDir);
    blocksDefaults.loaded = std::move(loaded);

    sortIds(blocksDefaults);

    if (!config.atlasRegeneration && tryLoadSavedAtlas(blocksDefaults, atlasPngPath, atlasUvPath)) {
        std::cout << "Loaded saved atlas from disk: " << atlasPngPath.string() << "\n";
    } else {
        buildAtlasForBlocks(blocksDefaults, texturesDir, atlasPngPath, atlasUvPath);
    }

    Texture2D atlas = blocksDefaults.atlasTex;

    //////////////////////////////////////////////////////////////////////////
    /////////////////      WORLD CLASS INITIALIZATION        /////////////////
    //////////////////////////////////////////////////////////////////////////

    GameRules gamerules;

    GameModes currentGamemode;
    currentGamemode = SURVIVAL;

    World currentWorld("Default World", {0.0f, 9.5f, 2.0f});
    std::shared_ptr<World> worldPtr(&currentWorld, [](World*){});
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
    SetBlockDefaults(&blocksDefaults);

    currentWorld.MarkAllChunksDirty();

    currentWorld.SetChunkModifiedCallback([&](int32_t cx,int32_t cy,int32_t cz){
        currentWorld.MarkChunkAsDirty(cx, cy, cz);
    });
    
    currentWorld.FillBlocks(16, -2, 16, -16, -16, -16, BlockFillActions::SET, 4);
    currentWorld.FillBlocks(16, -1, 16, -16, -1, -16, BlockFillActions::SET, 2);
    currentWorld.FillBlocks(16, 0, 16, -16, 0, -16, BlockFillActions::SET, 1);
    currentWorld.FillBlocks(2, 0, 2, 2, 10, 14, BlockFillActions::SET, 9);


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

    Vector3 rotation = {0.0f, 0.0f, 0.0f};
    Vector3 movement = {0.0f, 0.0f, 0.0f};

    float zoom = GetMouseWheelMove() * 0.5f;

    static Vector2 sensitivity = { 0.001f, 0.001f };
    static float tickAccumulator = 0.0f;

    float accumulatorPlayer = 0.0f;

    bool isCreativeFlyEnabled = true;

    double accumulator = 0.0;
    double lastTime = GetTime();

    bool queuedBreak = false;
    bool queuedPlace = false;

    RaycastHit currentHit{};
    Ray currentRay{};

    int handedBlockId = 1;

    bool placingAllowed = true;
    bool breakingAllowed = true;

    int blockPlacingCooldown = 0;

    bool applyBlockPlacementRestrictions = true;

    MoveMode playerMovementMode = WALKING;
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
    int f3textsize = GetRenderHeight() / 50;
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
    int chatTextsize = GetRenderHeight() / 50;

    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            screenheight = GetRenderHeight();
            screenwidth = GetRenderWidth();
            f3textsize = GetRenderHeight() / 50;
            chatTextsize = GetRenderHeight() / 50;
        }
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
                    unloadAtlas(blocksDefaults);
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
                
                currentWorld.ClearRendered();
                // get camera position in int

                int32_t camCx = (int)std::floor(camera.position.x / (float)CHUNK_SIZE);
                int32_t camCy = (int)std::floor(camera.position.y / (float)CHUNK_SIZE);
                int32_t camCz = (int)std::floor(camera.position.z / (float)CHUNK_SIZE);

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

                    //------------Basic rights management-----------
                    if (currentGamemode == SPECTATOR) {
                        breakingAllowed = false;
                        placingAllowed = false;
                    } else {
                        breakingAllowed = true;
                        placingAllowed = true;
                    }
                    //-------------Update dirty chunks-----------
                    size_t dirtyBudget = MAX_DIRTY_CHUNKS_PER_FRAME;

                    currentWorld.ProcessDirtyQueue(
                        (int32_t)dirtyBudget,
                        [&](int32_t cx, int32_t cy, int32_t cz) {

                            auto chunk = currentWorld.GetChunkAt(cx, cy, cz);
                            if (!chunk) return;
                            if (!chunk->IsChunkDirty()) return;

                            chunk->SetState(ChunkState::Meshing);

                            if (chunk->IsChunkLoaded()) {
                                chunk->UnloadChunk();
                                chunk->UnmarkAsLoaded();
                            }

                            Model m = BuildModelForChunk(chunk, &currentWorld);

                            if (m.meshCount > 0) {
                                chunk->UpdateChunkModel(m);
                                if (atlas.id) {
                                    chunk->SetChunkMaterialTexture(atlas);
                                }
                                chunk->MarkAsLoaded();
                                chunk->UnmarkAsDirty();
                                chunk->SetState(ChunkState::Ready);
                            } else {
                                chunk->UpdateChunkModel(Model{0});
                                chunk->UnmarkAsLoaded();
                                chunk->SetState(ChunkState::Generated);
                            }
                        }
                    );
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

                    if (IsKeyPressed(KEY_F1)) {
                        if (!HideHUD) {
                            HideHUD = true;
                        } else {
                            HideHUD = false;
                        }
                    }

                    if (IsKeyPressed(KEY_F2)) {
                        const std::filesystem::path outDir = std::filesystem::path(genPath(config.gameDirectory, "screenshots/"));
                        std::filesystem::create_directories(outDir);

                        const auto now = std::chrono::system_clock::now();
                        const std::time_t t = std::chrono::system_clock::to_time_t(now);
                        
                        std::tm tm{};

                        localtime_r(&t, &tm);

                        std::ostringstream oss;
                        oss << "screenshot_"
                            << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S")
                        << ".png";

                        const std::filesystem::path filePath = outDir / oss.str();

                        TakeScreenshot(filePath.string().c_str());

                        std::cout << "Taken screenshot at " << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << std::endl;
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

                    if (IsKeyPressed(KEY_F5)) {
                        if (cameraMode != CAMERA_THIRD_PERSON) {
                            cameraMode = CAMERA_THIRD_PERSON;
                        } else {
                            cameraMode = CAMERA_FIRST_PERSON;
                        }
                    }

                    //---i know that these keybinds are not very conventionnal... but it's temporary
                    if (IsKeyPressed(KEY_MINUS)) {
                        handedBlockId += 1;
                    }

                    if (IsKeyPressed(KEY_EQUAL)) {
                        handedBlockId -= 1;
                    }

                    if (IsKeyPressed(KEY_T)) {
                        std::cout << "Chat opened" << std::endl;
                        IsMovementsEnabled = false;
                        IsChatOpened = true;
                    }

                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) queuedBreak = true;
                    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) queuedPlace = true;
                    if (IsKeyPressed(MOUSE_BUTTON_MIDDLE)) {
                        handedBlockId = currentHit.id;
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
                        accumulatorPlayer,
                        tickAccumulator,
                        frame_dt,
                        mouseDelta,
                        zoom,
                        IsMovementsEnabled,
                        IsMouseEnabled
                    );

                    double now = GetTime();
                    accumulator += (now - lastTime);
                    lastTime = now;

                    while (accumulator >= TICK_DT)
                    {
                        currentRay = GetMouseRay(
                            (Vector2){ (float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f },
                            camera
                        );
                        currentRay.direction = Vector3Normalize(currentRay.direction);

                        currentHit = UpdateRaycastingTick(currentRay, camera, queuedBreak, queuedPlace, worldPtr, handedBlockId, breakingAllowed, placingAllowed, applyBlockPlacementRestrictions, blockPlacingCooldown);

                        queuedBreak = false;
                        queuedPlace = false;

                        accumulator -= TICK_DT;
                    }   
                    
                }
                //------------------Drawing----------------------
                BeginDrawing();
                ClearBackground(backgroundColor);
                BeginMode3D(camera);
                //--------------3D Drawing-----------------------

                for (int ox = -renderDistance; ox <= renderDistance; ++ox) {
                    for (int oy = -renderDistance; oy <= renderDistance; ++oy) {
                        for (int oz = -renderDistance; oz <= renderDistance; ++oz) {
                            int cx = camCx + ox;
                            int cy = camCy + oy;
                            int cz = camCz + oz;

                            auto chunk = currentWorld.GetChunkAt(cx, cy, cz);
                            if (!chunk) {
                                auto chunk = std::make_shared<Chunk>(cx, cy, cz);
                                currentWorld.AddChunk(chunk);
                                currentWorld.MarkChunkAsDirty(cx, cy, cz);
                                continue;
                            }

                            if (!currentWorld.IsChunkLoaded(cx, cy, cz)) {
                                currentWorld.MarkChunkAsDirty(cx, cy, cz);
                                continue;
                            }

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

                if (currentHit.hit)
                {
                    Vector3 p = {
                        (float)currentHit.x + 0.5f,
                        (float)currentHit.y + 0.5f,
                        (float)currentHit.z + 0.5f
                    };
                    DrawCubeWires(p, 1.001f, 1.001f, 1.001f, (Color){ 220, 40, 40, 255 });
                }
                EndMode3D();

                //-----------------------------2D Drawing-----------------------------

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

                //-------------Targeted block info-------------
                //DrawText("Hello", )
                //----------------Chat-------------------------
                if (IsChatOpened && !HideHUD) {
                    DrawChat(screenheight, screenwidth, chatTextsize, chatContent);
                }

                //---------------Pause screen-----------------
                if (IsGamePaused) {
                    DrawPauseScreen(screenheight, screenwidth);
                } 
                EndDrawing();
                break;
            }

            case OPTIONS: {    
                OptionsAction action = DrawAndHandleOptions(screenwidth, screenheight, background, backgroundColor, backgroundColor);    
                if (action == OptionsAction::QUIT) {   
                    unloadAtlas(blocksDefaults);     
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

    unloadAtlas(blocksDefaults);
    UnloadTexture(atlas);
    UnloadTexture(background);
    CloseWindow();
}