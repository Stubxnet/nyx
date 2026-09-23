#pragma once

#include "raylib.h"
#include <memory>
#include <string>

#include "enum.hpp"

#include "../data/BlocksDefaults.cpp"

#include "../lib/BlockDefaults.hpp"
#include "../lib/Chunk.hpp"
#include "../lib/World.hpp"
#include "../lib/RaycastHit.hpp"
#include "../lib/RenderState.hpp"
#include "../lib/Body.hpp"

#include "../core/constants.hpp"

struct WindowState {
    int width = 0;
    int height = 0;
};

struct ResourceState {
    Texture2D background = {};
    Texture2D atlas = {};
    BlocksDefaults blocksDefaults = {};
};

struct WorldState {
    World world;

    WorldState()
        : world("", {0.0f, 0.0f, 0.0f}) {}

    WorldState(const std::string& name, const Vector3& spawn)
        : world(name, spawn) {}
};

struct CameraState {
    Camera3D camera = {};
    Body body = {};
    RenderState renderState = {};
    Vector3 rotation = { DEFAULT_ROTATION_X, DEFAULT_ROTATION_Y, DEFAULT_ROTATION_Z };
    Vector3 movement = { DEFAULT_MOVEMENT_X, DEFAULT_MOVEMENT_Y, DEFAULT_MOVEMENT_Z };
    float zoom = DEFAULT_ZOOM;
    float sensitivityX = DEFAULT_SENSITIVITY_X;
    float sensitivityY = DEFAULT_SENSITIVITY_Y;
    int cameraMode = CAMERA_FIRST_PERSON;
};

struct InputState {
    bool isGamePaused = DEFAULT_IS_GAME_PAUSED;
    bool isMouseEnabled = DEFAULT_IS_MOUSE_ENABLED;
    bool isMovementsEnabled = DEFAULT_IS_MOVEMENTS_ENABLED;
    bool isCreativeFlyEnabled = DEFAULT_IS_CREATIVE_FLY_ENABLED;

    bool queuedBreak = false;
    bool queuedPlace = false;

    bool placingAllowed = DEFAULT_PLACING_ALLOWED;
    bool breakingAllowed = DEFAULT_BREAKING_ALLOWED;
    bool applyBlockPlacementRestrictions = DEFAULT_APPLY_BLOCK_PLACEMENT_RESTRICTIONS;

    int handedBlockId = DEFAULT_HANDED_BLOCK_ID;
    int blockPlacingCooldown = DEFAULT_BLOCK_PLACING_COOLDOWN;

    float reach = DEFAULT_REACH_SURVIVAL;
    double accumulator = DEFAULT_ACCUMULATOR;
    double lastTime = 0.0;

    float accumulatorPlayer = DEFAULT_ACCUMULATOR_PLAYER;

    float tickAccumulator = 0.0f;
    RaycastHit currentHit{};
};

struct HudState {
    bool hideHUD = DEFAULT_HIDE_HUD;
    bool f3enabled = true;
    bool drawBoundingBoxes = false;
    bool drawChunksGrid = false;

    int fps = 0;
    Color fpsColor = GREEN;
    Color f3color = RAYWHITE;

    int textSize = 0;
    int textSpacing = 0;
    int lineSize = 0;

    bool isChatOpened = DEFAULT_IS_CHAT_OPENED;
    std::string chatContent;
    int chatTextSize = 0;
};

struct GameState {
    WindowState window;
    ResourceState resources;
    WorldState world;
    CameraState camera;
    InputState input;
    HudState hud;

    GameScreen currentScreen = DEFAULT_GAME_SCREEN;
    GameModes currentGamemode = DEFAULT_GAMEMODE;
    MoveMode playerMovementMode = DEFAULT_PLAYER_MOVEMENT_MODE;

    int renderDistance = DEFAULT_RENDER_DISTANCE;
    int targetFPS = DEFAULT_TARGET_FPS;

    bool shouldExit = false;
};
