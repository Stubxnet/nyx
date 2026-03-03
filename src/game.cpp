#include "game.hpp"

enum GameScreen { MENU, GAME, OPTIONS };

void Game::init() {
    // TODO: use this initialization function
}

World Game::initGameData(World& world) { // only for tests
    GameRules defaultGamerules;

    Dimension overworld("Overworld");

    for (int x = -1; x <= 1; ++x) {
        for (int y = -1; y <= 1; ++y) {
            for (int z = -1; z <= 1; ++z) {
                Chunk chunk(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), "volcano", "desert");
                for (int bx = 0; bx < 16; ++bx) {
                    for (int by = 0; by < 16; ++by) {
                        for (int bz = 0; bz < 16; ++bz) {
                            Block block(bx + (x * 16), by + (y * 16), bz + (z * 16), 4);
                            block.setBreakable(true);
                            block.setResistance(2);
                            chunk.addBlock(block);
                        }
                    }
                }
                overworld.addChunk(chunk);
            }
        }
    }

    world.addDimension(overworld);
    world.setDefaultGamerules(defaultGamerules);
    return world;
}

void Game::run(const Config& config) {
    const char* title = config.windowTitle.c_str();
    InitWindow(config.windowWidth, config.windowHeight, title);

    GameScreen currentScreen = MENU;
    SetTargetFPS(60);

    //////////////////////////////////////////////////////////////////////////
    /////////////////            TEXTURES LOADING            /////////////////
    //////////////////////////////////////////////////////////////////////////

    Texture2D background = LoadTexture(genPath(config.gameDirectory, 
                  "assets/textures/background/background1920x1080p.png").c_str());

    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model cubeModel = LoadModelFromMesh(cubeMesh);
    Model cubeModel2 = LoadModelFromMesh(cubeMesh);
    Model cubeModel3 = LoadModelFromMesh(cubeMesh);
    Vector3 cubePosition = { 0.0f, 0.0f, 0.0f };
    
    Image cubeTextureImage = LoadImage(genPath(config.gameDirectory, "assets/textures/blocks/cube_texture.png").c_str());
    Image cubeTextureImage2 = LoadImage(genPath(config.gameDirectory, "assets/textures/blocks/textures2.png").c_str());
    Image cubeTextureImage3 = LoadImage(genPath(config.gameDirectory, "assets/textures/blocks/stone.png").c_str());
    Texture2D texture = LoadTextureFromImage(cubeTextureImage);
    Texture2D texture2 = LoadTextureFromImage(cubeTextureImage2);
    Texture2D texture3 = LoadTextureFromImage(cubeTextureImage3);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    cubeModel2.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture2;
    cubeModel3.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture3;

    UnloadImage(cubeTextureImage);
    UnloadImage(cubeTextureImage2);
    UnloadImage(cubeTextureImage3);

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

    float player_speed = 0.5f;                // TODO: Define a class Entity and create an instance for the player
    Vector3 rotation = {0.0f, 0.0f, 0.0f};

    float zoom = GetMouseWheelMove() * 0.5f;

    int ZQSD_or_WASD = 1; // put it on 0 to use WASD and on 1 to use ZQSD

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

    // F1
    int HideHUD = false;

    // Chat
    int IsChatOpened = false;

    while (!WindowShouldClose()) {
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
                rotation.x = GetMouseDelta().x * 0.1f;
                rotation.y = GetMouseDelta().y * 0.1f;

                Vector3 movement = {0.0f, 0.0f, 0.0f};
                // ---------- Cursor management
                if (!IsCursorHidden()) {
                    DisableCursor();
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
                if (IsKeyPressed(KEY_P)) {
                    if (camera.projection == CAMERA_PERSPECTIVE) {
                        cameraMode = CAMERA_THIRD_PERSON;
                        camera.position = (Vector3){ 0.0f, 2.0f, -100.0f };
                        camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
                        camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
                        camera.projection = CAMERA_ORTHOGRAPHIC;
                        camera.fovy = 20.0f;
                        CameraYaw(&camera, -135 * DEG2RAD, true);
                        CameraPitch(&camera, -45 * DEG2RAD, true, true, false);
                    } else if (camera.projection == CAMERA_ORTHOGRAPHIC) {
                        cameraMode = CAMERA_THIRD_PERSON;
                        camera.position = (Vector3){ 0.0f, 2.0f, 10.0f };
                        camera.target = (Vector3){ 0.0f, 2.0f, 0.0f };
                        camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };
                        camera.projection = CAMERA_PERSPECTIVE;
                        camera.fovy = 60.0f;
                    }
                }
                if (IsKeyPressed(KEY_T)) {
                    std::cout << "Chat opened" << std::endl;
                    IsChatOpened = true;
                }
                if (ZQSD_or_WASD == 0) {
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
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        movement.z = -player_speed;
                    }
                } else if (ZQSD_or_WASD == 1) {
                    if (IsKeyDown(KEY_Z)) {
                        movement.x = player_speed;
                    }
                    if (IsKeyDown(KEY_S)) {
                        movement.x = -player_speed;
                    }
                    if (IsKeyDown(KEY_Q)) {
                        movement.y = player_speed;
                    }
                    if (IsKeyDown(KEY_D)) {
                        movement.y = -player_speed;
                    }
                    if (IsKeyDown(KEY_SPACE)) {
                        movement.z = player_speed;
                    }
                    if (IsKeyDown(KEY_LEFT_SHIFT)) {
                        movement.z = -player_speed;
                    }
                }
                
                UpdateCameraPro(&camera, movement, rotation, zoom);
                
                BeginDrawing();
                ClearBackground(backgroundColor);
                BeginMode3D(camera);
                //--------------3D Drawing-----------------------
                DrawModel(cubeModel, cubePosition, 1.0f, WHITE);
                DrawModel(cubeModel, (Vector3){2.0f, 2.0f, 2.0f}, 1.0f, WHITE);
                DrawModel(cubeModel2, (Vector3){2.0f, 3.0f, 2.0f}, 1.0f, WHITE);
                DrawModel(cubeModel3, (Vector3){2.0f, 4.0f, 2.0f}, 1.0f, WHITE);


                if (cameraMode == CAMERA_THIRD_PERSON) {
                    DrawCube(camera.target, 0.5f, 0.5f, 0.5f, PURPLE);
                    DrawCubeWires(camera.target, 0.5f, 0.5f, 0.5f, DARKPURPLE);
                }

                EndMode3D();

                //-------------------2D Drawing--------------------

                if (f3enabled && !HideHUD) {
                    DrawText("Nyx build pre-release 1.0.0", 15, f3textSpacing, f3textsize, RAYWHITE);
                    DrawText("Camera controls:", 15, f3textSpacing + f3lineSize, f3textsize, RAYWHITE);
                    DrawText("- W, A, S, D, Space, Left-Ctrl to move", 15, f3textSpacing + f3lineSize*2, f3textsize, RAYWHITE);
                    DrawText("- Arrow keys or mouse to look around", 15, f3textSpacing + f3lineSize*3, f3textsize, RAYWHITE);
                    DrawText("- Camera mode keys: 1, 2, 3, 4", 15, f3textSpacing + f3lineSize*4, f3textsize, RAYWHITE);
                    DrawText("- Zoom keys: num-plus, num-minus or mouse scroll", 15, f3textSpacing + f3lineSize*5, f3textsize, RAYWHITE);
                    DrawText("- Camera projection key: P", 15, f3textSpacing + f3lineSize*6, f3textsize, RAYWHITE);
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
                        UnloadModel(cubeModel);
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

    UnloadModel(cubeModel);
    UnloadTexture(background);
    CloseWindow();
}
