#include "raylib.h"
#include "raymath.h"
#include <memory>

#include "../lib/RaycastHit.hpp"
#include "../lib/DDA_raycasting.cpp"
#include "../lib/World.hpp"
#include "../constants.hpp"
 
//------------------------Raycasting update--------------------------
RaycastHit UpdateRaycastingTick(
    const Ray& ray,
    bool queuedBreak,
    bool queuedPlace,
    const std::shared_ptr<World>& world, 
    int placingId,
    bool breakingAllowed,
    bool placingAllowed
) {
    RaycastHit hit = DDA_RaycastWorld(ray, world, 200.0f);
    if (!world) return hit;

    if (hit.hit && queuedBreak && breakingAllowed)
    {
        world->SetBlock(hit.x, hit.y, hit.z, 0, SetblockActions::SET);
    }

    if (hit.hit && queuedPlace && placingAllowed)
    {
        int64_t px = hit.x + hit.normalX;
        int64_t py = hit.y + hit.normalY;
        int64_t pz = hit.z + hit.normalZ;

        if (world->GetBlockId(px, py, pz) == 0)
        {
            world->SetBlock(px, py, pz, placingId, SetblockActions::SET);
        }
    }

    return hit;
}

//---------------------Player update---------------------------------------
inline float ClampFloat(float v, float lo, float hi) {
    return (v < lo) ? lo : (v > hi) ? hi : v;
}

static BoundingBox CreatePlayerHitbox(const Camera3D &camera) {
    const float eyeX = camera.position.x;
    const float eyeY = camera.position.y;
    const float eyeZ = camera.position.z;

    const float bottomY = eyeY - EYES_Y;
    const float halfY = STAND_HEIGHT * 0.5f;
    const float centerY = bottomY + halfY;

    const float halfX = BOTTOM_HEIGHT / 2.0f;
    const float halfZ = BOTTOM_HEIGHT * 0.5f;

    const float centerX = eyeX;
    const float centerZ = eyeZ;

    return {
        {centerX - halfX, centerY - halfY, centerZ - halfZ},  // min
        {centerX + halfX, centerY + halfY, centerZ + halfZ}   // max
    };
}

template<typename WorldType>
static bool HitboxIntersectsSolidAtCamera(const Camera3D &camera,
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
                int bid = world.GetBlockId(x, y, z); // returns 0 if chunk or block missing
                if (bid != 0) {
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
static bool TryStepUpAndResolve(
    Camera3D &tempCam,
    const Vector3 &origMove,
    Vector3 &movementOut,
    WorldType &world,
    float stepHeight
) {
    tempCam.position.y += stepHeight;
    BoundingBox steppedBox = CreatePlayerHitbox(tempCam);
    if (!HitboxIntersectsSolidAtCamera(tempCam, steppedBox, world)) {
        return true;
    } else {
        // rollback y
        tempCam.position.y -= stepHeight;
        return false;
    }
}

template<typename WorldType, typename GamemodeType>
static bool ResolveCollisions(
    Vector3 &movement,
    Camera3D &tempCam,
    WorldType &world,
    Body &body,
    const GamemodeType &currentGamemode,
    const GamemodeType &SPECTATOR
) {
    bool collided = false;
    if (currentGamemode == SPECTATOR) return false;

    constexpr float STEP_HEIGHT = 0.6f;

    // Vertical first (Y)
    if (movement.y != 0.0f) {
        tempCam.position.y += movement.y;
        BoundingBox playerBox = CreatePlayerHitbox(tempCam);
        if (HitboxIntersectsSolidAtCamera(tempCam, playerBox, world)) {
            tempCam.position.y -= movement.y;
            if (movement.y < 0.0f) {
                body.OnGround = true;
            }
            movement.y = 0.0f;
            collided = true;
        } else {
            // no vertical collision: if moving downwards, not necessarily on ground yet
            if (movement.y < 0.0f) {
                // still possibly falling; OnGround remains as-is
            }
        }
    }

    // Determine horizontal collision order using 1.14+ rule:
    // if |velZ| > |velX| => Y-X-Z, else Y-Z-X
    float absVx = fabsf(body.velocity.x);
    float absVz = fabsf(body.velocity.z);

    auto tryMoveAxis = [&](char axis) -> void {
        if (axis == 'X') {
            if (movement.x == 0.0f) return;

            Vector3 origCamPos{ tempCam.position.x, tempCam.position.y, tempCam.position.z };

            tempCam.position.x += movement.x;
            BoundingBox playerBox = CreatePlayerHitbox(tempCam);
            if (HitboxIntersectsSolidAtCamera(tempCam, playerBox, world)) {
                if (body.OnGround) {
                    float origY = tempCam.position.y;

                    // rollback x first (so stepping can retry the same axis movement)
                    tempCam.position.x = origCamPos.x;

                    // try step up from the rolled-back x position
                    if (TryStepUpAndResolve<WorldType, GamemodeType>(tempCam, movement, movement, world, STEP_HEIGHT)) {
                        // now retry the X movement at the stepped height
                        tempCam.position.x = origCamPos.x + movement.x;

                        BoundingBox steppedBox = CreatePlayerHitbox(tempCam);
                        if (HitboxIntersectsSolidAtCamera(tempCam, steppedBox, world)) {
                            // stepping didn't clear collision -> rollback axis and y
                            tempCam.position.x = origCamPos.x;
                            tempCam.position.y = origY;
                            movement.x = 0.0f;
                            collided = true;
                        } else {
                            collided = true;
                        }
                    } else {
                        // cannot step: keep original x rollback
                        tempCam.position.x = origCamPos.x;
                        movement.x = 0.0f;
                        collided = true;
                    }
                } else {
                    // not on ground -> simple rollback
                    tempCam.position.x = origCamPos.x;
                    movement.x = 0.0f;
                    collided = true;
                }
            }
        } else if (axis == 'Z') {
            if (movement.z == 0.0f) return;

            Vector3 origCamPos{ tempCam.position.x, tempCam.position.y, tempCam.position.z };

            tempCam.position.z += movement.z;
            BoundingBox playerBox = CreatePlayerHitbox(tempCam);
            if (HitboxIntersectsSolidAtCamera(tempCam, playerBox, world)) {
                if (body.OnGround) {
                    float origY = tempCam.position.y;

                    // rollback z first (so stepping can retry the same axis movement)
                    tempCam.position.z = origCamPos.z;

                    // try step up from the rolled-back z position
                    if (TryStepUpAndResolve<WorldType, GamemodeType>(tempCam, movement, movement, world, STEP_HEIGHT)) {
                        // now retry the Z movement at the stepped height
                        tempCam.position.z = origCamPos.z + movement.z;

                        BoundingBox steppedBox = CreatePlayerHitbox(tempCam);
                        if (HitboxIntersectsSolidAtCamera(tempCam, steppedBox, world)) {
                            // stepping didn't clear collision -> rollback axis and y
                            tempCam.position.z = origCamPos.z;
                            tempCam.position.y = origY;
                            movement.z = 0.0f;
                            collided = true;
                        } else {
                            collided = true;
                        }
                    } else {
                        // cannot step: keep original z rollback
                        tempCam.position.z = origCamPos.z;
                        movement.z = 0.0f;
                        collided = true;
                    }
                } else {
                    // not on ground -> simple rollback
                    tempCam.position.z = origCamPos.z;
                    movement.z = 0.0f;
                    collided = true;
                }
            }
        }
    };

    if (absVz > absVx) {
        tryMoveAxis('X');
        tryMoveAxis('Z');
    } else {
        tryMoveAxis('Z');
        tryMoveAxis('X');
    }

    return collided;
}

void UpdateBodyTick(
    Body &body,
    const Vector3 &direction,
    bool jumpPressed,
    bool isSprinting,
    bool isSneaking
) {
    // gravity
    if (body.OnGround) {
        if (jumpPressed) {
            body.velocity.y = JUMP_VELO;
            body.OnGround = false;
        }
    } else {
        body.velocity.y -= GRAVITY;
    }

    // movement params
    const float baseSpeed = BASE_MOVE_SPEED;
    float moveSpeed = baseSpeed;
    if (isSprinting) moveSpeed *= SPRINT_MULTIPLIER;
    if (isSneaking) moveSpeed *= SNEAK_MULTIPLIER;

    // acceleration model
    const float accel = moveSpeed * 0.5f; // acceleration factor (tunable)
    const float maxSpeed = moveSpeed * 1.5f; // clamp horizontal speed

    // apply acceleration to horizontal velocity
    body.velocity.x += direction.x * accel;
    body.velocity.z += direction.z * accel;

    // clamp horizontal speed
    float horizSpeed = sqrtf(body.velocity.x*body.velocity.x + body.velocity.z*body.velocity.z);
    if (horizSpeed > maxSpeed) {
        float scale = maxSpeed / horizSpeed;
        body.velocity.x *= scale;
        body.velocity.z *= scale;
    }

    // friction / drag
    if (body.OnGround) {
        body.velocity.x *= GROUND_FRICTION;
        body.velocity.z *= GROUND_FRICTION;
    } else {
        body.velocity.x *= AIR_DRAG;
        body.velocity.z *= AIR_DRAG;
        body.velocity.y *= 1.0f;
    }

    if (fabsf(body.velocity.x) < 1e-5f) body.velocity.x = 0.0f;
    if (fabsf(body.velocity.z) < 1e-5f) body.velocity.z = 0.0f;

    // integrate position
    body.position.x += body.velocity.x;
    body.position.y += body.velocity.y;
    body.position.z += body.velocity.z;
}

void UpdateMovementFly(
    Camera3D &camera,
    Vector3 &movement,
    const Vector2 &mouseDelta,
    Vector3 &rotation,
    float zoom,
    float dt,
    bool IsMovementsEnabled
) {
    movement = {0.0f, 0.0f, 0.0f};

    Vector3 localMove = {0.0f, 0.0f, 0.0f};

    if (IsMovementsEnabled) {
        if (IsKeyDown(KEY_W)) localMove.z -= 1.0f; // forward
        if (IsKeyDown(KEY_S)) localMove.z += 1.0f; // back
        if (IsKeyDown(KEY_D)) localMove.x -= 1.0f; // right
        if (IsKeyDown(KEY_A)) localMove.x += 1.0f; // left
        if (IsKeyDown(KEY_SPACE)) localMove.y += 1.0f; // up
        if (IsKeyDown(KEY_LEFT_SHIFT)) localMove.y -= 1.0f; // down
    }

    // normalize horizontal plane
    float len = sqrtf(localMove.x*localMove.x + localMove.z*localMove.z);
    if (len > 0.0f) {
        localMove.x /= len;
        localMove.z /= len;
    }

    const float flySpeed = 0.2f + zoom * 0.05f;
    // compute forward/right vectors from yaw/pitch (rotation)
    float yaw = rotation.x;
    float cosYaw = cosf(yaw);
    float sinYaw = sinf(yaw);

    // convert localMove to world movement (x,z)
    movement.x = (localMove.x * cosYaw) + (localMove.z * sinYaw);
    movement.z = (localMove.x * -sinYaw) + (localMove.z * cosYaw);
    movement.y = localMove.y;

    // scale
    movement.x *= flySpeed;
    movement.y *= flySpeed;
    movement.z *= flySpeed;

    // apply camera update immediately for fly (no collisions)
    Vector3 forward = { -sinf(rotation.x), 0.0f, -cosf(rotation.x) };
    Vector3 up = { 0.0f, 1.0f, 0.0f };

    Vector3 viewDir = Vector3Scale(forward, cosf(rotation.y));
    viewDir = Vector3Add(viewDir, Vector3Scale(up, sinf(rotation.y)));

    camera.position = Vector3Add(camera.position, movement);
    camera.target = Vector3Add(camera.position, viewDir);
    camera.up = up;
}

template<typename WorldType, typename GamemodeType>
void UpdatePlayerMovementTick(
    Camera3D &camera,
    Body &body,
    RenderState &renderState,
    WorldType &world,
    const GamemodeType &currentGamemode,
    bool normalModeFlag,
    Vector3 &movement,
    Vector3 &rotation,
    float &accumulator,
    float &tickAccumulator,
    float dt,
    const Vector2 &mouseDelta,
    float zoom,
    bool IsMovementsEnabled,
    bool IsMouseEnabled
) {

    if (dt > 0.25f) dt = 0.25f;
    accumulator += dt;

    const bool isFlyMode = (currentGamemode == SPECTATOR) ||
                           (currentGamemode == BUILDER) ||
                           (normalModeFlag && currentGamemode == CREATIVE);

    if (isFlyMode) {
        UpdateMovementFly(camera, movement, mouseDelta, rotation, zoom, dt, IsMovementsEnabled);
        renderState.previousCameraPosition = renderState.currentCameraPosition;
        renderState.currentCameraPosition = camera.position;
        body.position = camera.position - Vector3{0, EYES_Y, 0};
        return;
    }

    Vector2 inputDir = {0.0f, 0.0f};
    if (IsMovementsEnabled) {
        if (IsKeyDown(KEY_W)) inputDir.y -= 1.0f;
        if (IsKeyDown(KEY_S)) inputDir.y += 1.0f;
        if (IsKeyDown(KEY_D)) inputDir.x -= 1.0f;
        if (IsKeyDown(KEY_A)) inputDir.x += 1.0f;
    }

    if ((inputDir.x != 0.0f) && (inputDir.y != 0.0f)) {
        inputDir = Vector2Scale(inputDir, 1.0f / sqrtf(2.0f));
    }

    bool isSprinting = IsKeyDown(KEY_LEFT_SHIFT) && (inputDir.y > 0.0f);
    bool isSneaking = IsKeyDown(KEY_LEFT_CONTROL);

    bool jumpPressedFrame = IsKeyDown(KEY_SPACE);

    while (accumulator >= TICK_DT) {
        bool jumpConsumed = jumpPressedFrame;
        jumpPressedFrame = false;

        float sinYaw = sinf(rotation.x);
        float cosYaw = cosf(rotation.x);
        Vector3 direction = {
            inputDir.x * cosYaw + inputDir.y * sinYaw, // x
            0.0f,
            inputDir.x * (-sinYaw) + inputDir.y * cosYaw // z
        };

        UpdateBodyTick(body, direction, jumpConsumed, isSprinting, isSneaking);

        Vector3 intendedEyePos = Vector3Add(body.position, (Vector3){0.0f, (isSneaking ? SNEAK_EYES_Y : EYES_Y), 0.0f});
        Vector3 prevEyePos = renderState.previousCameraPosition;
        Vector3 deltaMovement = Vector3Subtract(intendedEyePos, prevEyePos);

        Camera3D tempCam = camera;
        tempCam.position = prevEyePos;
        bool hit = ResolveCollisions(deltaMovement, tempCam, world, body, currentGamemode, SPECTATOR);
                
        BoundingBox footBox = CreatePlayerHitbox(tempCam);
        const float checkDepth = 0.05f;
        footBox.max.y = footBox.min.y + checkDepth;
        if (!HitboxIntersectsSolidAtCamera(tempCam, footBox, world)) {
            body.OnGround = false;
        }

        // If vertical collision occured (deltaMovement.y == 0 after attempted move while moving down),
        // UpdateBodyTick landing was handled in ResolveCollisions via body.OnGround set inside.
        if (deltaMovement.y == 0.0f && body.velocity.y < 0.0f) {
            body.velocity.y = 0.0f;
            body.OnGround = true;
            float eyeHeight = (isSneaking ? SNEAK_EYES_Y : EYES_Y);
            body.position.y = tempCam.position.y - eyeHeight;
        }

        camera.position = tempCam.position;
        float eyeHeight = (isSneaking ? SNEAK_EYES_Y : EYES_Y);
        body.position.x = camera.position.x;
        body.position.z = camera.position.z;
        if (!body.OnGround) {
            body.position.y = camera.position.y - eyeHeight;
        }

        accumulator -= TICK_DT;
        tickAccumulator = 0.0f;
    }

    tickAccumulator += dt;
    float lerpFactor = ClampFloat(tickAccumulator / TICK_DT, 0.0f, 1.0f);
    Vector3 interpolatedPos = Vector3Lerp(
        renderState.previousCameraPosition,
        Vector3Add(body.position, (Vector3){0.0f, (IsKeyDown(KEY_LEFT_CONTROL) ? SNEAK_EYES_Y : EYES_Y), 0.0f}),
        lerpFactor
    );

    camera.position = interpolatedPos;

    Vector3 forward = { -sinf(rotation.x), 0.0f, -cosf(rotation.x) };
    Vector3 up = { 0.0f, 1.0f, 0.0f };

    Vector3 viewDir = Vector3Scale(forward, cosf(rotation.y));
    viewDir = Vector3Add(viewDir, Vector3Scale(up, sinf(rotation.y)));

    camera.target = Vector3Add(camera.position, viewDir);
    camera.up = up;

    renderState.currentCameraPosition = camera.position;
}
