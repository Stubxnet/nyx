#pragma once
#include "raylib.h"
#include "enum.hpp"
#include <cstddef>

#define TICK_RATE 20.0f
#define TICK_DT (1.0f / TICK_RATE)
#define BLOCK_PLACING_TICK_COOLDOWN 10

#define GRAVITY 0.08f
#define JUMP_VELO 0.52f
#define AIR_DRAG 0.98f
#define SLIPPERINESS 0.6f
#define GROUND_FRICTION (SLIPPERINESS * 0.91f)
#define BASE_MOVE_SPEED 0.1f

#define SPRINT_MULTIPLIER 1.3f
#define SNEAK_MULTIPLIER 0.3f
#define CROUCH_HEIGHT 1.5f
#define STAND_HEIGHT 1.85f
#define BOTTOM_HEIGHT 0.5f

#define EYES_Y 1.50f
#define SNEAK_EYES_Y 1.25f

constexpr int TILE = 32;

constexpr int CHUNK_SIZE = 16;
constexpr size_t MAX_DIRTY_CHUNKS_PER_FRAME = 18;

// Defaults
constexpr int DEFAULT_RENDER_DISTANCE = 2;
constexpr int DEFAULT_TARGET_FPS = 60;
constexpr GameScreen DEFAULT_GAME_SCREEN = MENU;
constexpr GameModes DEFAULT_GAMEMODE = SURVIVAL;
constexpr MoveMode DEFAULT_PLAYER_MOVEMENT_MODE = WALKING;

constexpr bool DEFAULT_CHECKBLOCK = true;
constexpr bool DEFAULT_IS_CREATIVE_FLY_ENABLED = true;
constexpr bool DEFAULT_IS_GAME_PAUSED = false;
constexpr bool DEFAULT_IS_MOUSE_ENABLED = false;
constexpr bool DEFAULT_IS_MOVEMENTS_ENABLED = true;
constexpr bool DEFAULT_HIDE_HUD = false;
constexpr bool DEFAULT_IS_CHAT_OPENED = false;

constexpr int DEFAULT_HANDED_BLOCK_ID = 1;
constexpr bool DEFAULT_PLACING_ALLOWED = true;
constexpr bool DEFAULT_BREAKING_ALLOWED = true;
constexpr int DEFAULT_BLOCK_PLACING_COOLDOWN = 0;
constexpr bool DEFAULT_APPLY_BLOCK_PLACEMENT_RESTRICTIONS = true;

constexpr float DEFAULT_REACH_SURVIVAL = 5.0f;
constexpr float DEFAULT_REACH_BUILDER = 255.0f;

constexpr float DEFAULT_SENSITIVITY_X = 0.001f;
constexpr float DEFAULT_SENSITIVITY_Y = 0.001f;

constexpr float DEFAULT_ROTATION_X = 0.0f;
constexpr float DEFAULT_ROTATION_Y = 0.0f;
constexpr float DEFAULT_ROTATION_Z = 0.0f;

constexpr float DEFAULT_MOVEMENT_X = 0.0f;
constexpr float DEFAULT_MOVEMENT_Y = 0.0f;
constexpr float DEFAULT_MOVEMENT_Z = 0.0f;

constexpr float DEFAULT_ZOOM = 0.0f;
constexpr float DEFAULT_ACCUMULATOR_PLAYER = 0.0f;
constexpr double DEFAULT_ACCUMULATOR = 0.0;
