#pragma once

#include "raylib.h"
#include "../constants.hpp"

static BoundingBox CreatePlayerHitbox(const Camera3D &camera) {
    const float eyeX = camera.position.x;
    const float eyeY = camera.position.y;
    const float eyeZ = camera.position.z;

    const float bottomY = eyeY - EYES_Y;
    const float halfY = BODY_HEIGHT * 0.5f;
    const float centerY = bottomY + halfY;

    const float halfX = HALF_WIDTH;
    const float halfZ = BODY_WIDTH * 0.5f;

    const float centerX = eyeX;
    const float centerZ = eyeZ;

    return {
        {centerX - halfX, centerY - halfY, centerZ - halfZ},  // min
        {centerX + halfX, centerY + halfY, centerZ + halfZ}   // max
    };
}

template<typename WorldType>
static bool HitboxIntersectsSolid(const Camera3D &camera, 
    const BoundingBox &playerBox,
    WorldType &world) {
        constexpr float eps = 1e-6f;
        const int bx0 = (int)std::floor(playerBox.min.x);
        const int bx1 = (int)std::floor(playerBox.max.x - eps);
        const int by0 = (int)std::floor(playerBox.min.y);
        const int by1 = (int)std::floor(playerBox.max.y - eps);
        const int bz0 = (int)std::floor(playerBox.min.z);
        const int bz1 = (int)std::floor(playerBox.max.z - eps);

        for (int x = bx0; x <= bx1; ++x) {
            for (int y = by0; y <= by1; ++y) {
                for (int z = bz0; z <= bz1; ++z) {
                    if (world.GetBlockId(x, y, z) != 0) {
                        BoundingBox blockBox = {
                            {(float)x, (float)y, (float)z},
                            {(float)x + 1.0f, (float)y + 1.0f, (float)z + 1.0f}
                        };

                        if (CheckCollisionBoxes(playerBox, blockBox)) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

template<typename WorldType, typename GamemodeType>
static bool ResolveCollisions(Vector3 &movement,
    Camera3D &tempCam,
    const BoundingBox &playerBox,
    WorldType &world,
    const GamemodeType &currentGamemode,
    const GamemodeType &SPECTATOR) {
        bool collided = false;

    
        if (currentGamemode != SPECTATOR) {
            // X
            if (movement.x != 0.0f) {
                tempCam.position.x += movement.x;
                if (HitboxIntersectsSolid(tempCam, playerBox, world)) {
                    tempCam.position.x -= movement.x;
                    movement.x = 0.0f;
                    collided = true;
                }
            }
            // Y
            if (movement.y != 0.0f) {
                tempCam.position.y += movement.y;
                if (HitboxIntersectsSolid(tempCam, playerBox, world)) {
                    tempCam.position.y -= movement.y;
                    movement.y = 0.0f;
                    collided = true;
                }
            }
            // Z
            if (movement.z != 0.0f) {
                tempCam.position.z += movement.z;
                if (HitboxIntersectsSolid(tempCam, playerBox, world)) {
                    tempCam.position.z -= movement.z;
                    movement.z = 0.0f;
                    collided = true;
                }
            }
        }

        return collided;
    }

static void DrawPlayerHitbox(const Camera3D &camera, bool drawBoundingBoxes) {
    if (drawBoundingBoxes && camera.projection == CAMERA_THIRD_PERSON) {
        BoundingBox playerBox = CreatePlayerHitbox(camera);
        DrawBoundingBox(playerBox, LIME);
    }
}
