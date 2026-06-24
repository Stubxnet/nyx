#pragma once
#include "raylib.h"

#define TICK_RATE 20.0f
#define TICK_DT (1.0f / TICK_RATE)

#define GRAVITY 0.08f
#define JUMP_VELO 0.42f
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

// Data structures

// Player tick update data
struct Body {
    Vector3 position;
    Vector3 velocity;
    Vector3 dir;
    bool OnGround;
};

// actual and previous camera position
struct RenderState {
    Vector3 previousCameraPosition;
    Vector3 currentCameraPosition;
};


// Atlas parameters
const int TILE = 32;
const int ATLAS_COLS = 4;
const int ATLAS_ROWS = 4;

constexpr int CHUNK_SIZE = 16;