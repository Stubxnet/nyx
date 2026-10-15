#pragma once
#include "raylib.h"
#include "enum.hpp"
#include <cstddef>

/// ticks
constexpr float TICK_RATE = 20.0f;
constexpr float TICK_DT = (1.0f / TICK_RATE);
constexpr int BLOCK_PLACING_TICK_COOLDOWN = 10;

// physics
constexpr float GRAVITY = 0.08f;
constexpr float JUMP_VELO = 0.52f;
constexpr float AIR_DRAG = 0.98f;
constexpr float SLIPPERINESS = 0.6f;
constexpr float GROUND_FRICTION = (SLIPPERINESS * 0.91f);

// player stats
constexpr float BASE_MOVE_SPEED = 0.1f;
constexpr float SPRINT_MULTIPLIER = 1.3f;
constexpr float SNEAK_MULTIPLIER = 0.3f;
constexpr float CROUCH_HEIGHT = 1.5f;
constexpr float STAND_HEIGHT = 1.85f;
constexpr float BOTTOM_HEIGHT = 0.5f;
constexpr float EYES_Y = 1.50f;
constexpr float SNEAK_EYES_Y = 1.25f;

// texture atlas tile size
constexpr int TILE = 32;

// chunks
constexpr int CHUNK_SIZE = 16;
constexpr size_t MAX_DIRTY_CHUNKS_PER_FRAME = 18;

// default data for some paramters
constexpr int DEFAULT_RENDER_DISTANCE = 2;
constexpr int DEFAULT_TARGET_FPS = 60;
constexpr GameScreen DEFAULT_GAME_SCREEN = MENU;
constexpr GameModes DEFAULT_GAMEMODE = SURVIVAL;
constexpr MoveMode DEFAULT_PLAYER_MOVEMENT_MODE = WALKING;

// perhaps these ones aren't needed but...
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

// both of these should be editable in the future
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

//////////////////////
// World generation //
//////////////////////

// generic world parmeters
constexpr int32_t SEA_LEVEL = 0;

constexpr int32_t MIN_SURFACE_Y = -128;
constexpr int32_t MAX_SURFACE_Y = 512;

constexpr int32_t MIN_WORLD_Y = -32768;

// -----------------------------------------------------------------------------
// world generation
// -----------------------------------------------------------------------------

constexpr uint32_t DEFAULT_SEED = 1337;

constexpr int GENERATION_BUDGET = 1;

constexpr float WORLD_SCALE = 16.0f;

// -----------------------------------------------------------------------------
// continentalness
// -----------------------------------------------------------------------------

constexpr float SEA_CONTINENTALNESS = 0.50f;

constexpr float COAST_START = 0.40f;
constexpr float COAST_END   = 0.60f;

// -----------------------------------------------------------------------------
// terrain heights
// -----------------------------------------------------------------------------

constexpr float MAX_OCEAN_DEPTH_BLOCKS = 32.0f;
constexpr float MIN_OCEAN_DEPTH_BLOCKS = 3.0f;
constexpr float OCEAN_DETAIL_BLOCKS    = 1.25f;

constexpr float COASTAL_HEIGHT_BLOCKS = 10.0f;
constexpr float CONTINENTAL_HEIGHT_BLOCKS = 76.0f;

constexpr float MOUNTAIN_HEIGHT_BLOCKS = 120.0f;
constexpr float VALLEY_DEPTH_BLOCKS    = 22.0f;
constexpr float TERRAIN_DETAIL_BLOCKS  = 3.0f;

// -----------------------------------------------------------------------------
// domain warp
// -----------------------------------------------------------------------------

constexpr float WORLD_WARP_DISTANCE = 32.0f;

// -----------------------------------------------------------------------------
// noise frequencies
// WORLD_SCALE = 16 means that the effective wavelength in world blocks is
// approximately: wavelength = 1.0 / frequency / WORLD_SCALE
// -----------------------------------------------------------------------------

// frequencies for generators
constexpr float CONTINENT_FREQUENCY = 0.00010f;
constexpr float CLIMATE_FREQUENCY = 0.00025f;
constexpr float EROSION_FREQUENCY = 0.00030f;
constexpr float WEIRDNESS_FREQUENCY = 0.00028f;
constexpr float DETAIL_FREQUENCY = 0.00350f;
constexpr float WARP_FREQUENCY = 0.000125f;
constexpr float OCEAN_DEPTH_FREQUENCY = 0.00035f;
constexpr float OCEAN_DETAIL_FREQUENCY = 0.00150f;

