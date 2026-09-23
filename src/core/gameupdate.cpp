#include "gamestate.hpp"

#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

static void handleWindowEvents(GameState& gs) {
    if (IsWindowResized()) {
        gs.window.width = GetRenderWidth();
        gs.window.height = GetRenderHeight();
        gs.hud.textSize = gs.window.height / 50;
        gs.hud.textSpacing = gs.hud.textSize / 2;
        gs.hud.lineSize = gs.hud.textSpacing + gs.hud.textSize;
        gs.hud.chatTextSize = gs.window.height / 50;
    }

    if ((IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_RIGHT_CONTROL)) && IsKeyPressed(KEY_C)) {
        std::cout << "Pressed Ctrl+C. Exiting.\n";
        gs.shouldExit = true;
    }
}

static void handleEscape(GameState& gs) {
    if (!IsKeyPressed(KEY_ESCAPE)) return;

    if (gs.hud.isChatOpened) {
        gs.hud.isChatOpened = false;
        gs.hud.chatContent.clear();
        gs.input.isMovementsEnabled = true;
        return;
    }

    gs.input.isGamePaused = !gs.input.isGamePaused;
    if (gs.input.isGamePaused) {
        ShowCursor();
    } else {
        DisableCursor();
    }
}

static void handleChat(GameState& gs) {
    if (!gs.hud.isChatOpened) return;

    int key = 0;
    while ((key = GetCharPressed()) > 0) {
        if (key >= 32 && key <= 125) {
            gs.hud.chatContent += static_cast<char>(key);
        }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !gs.hud.chatContent.empty()) {
        gs.hud.chatContent.pop_back();
    }

    if (IsKeyPressed(KEY_ENTER)) {
        if (!gs.hud.chatContent.empty() && gs.hud.chatContent[0] == '/') {
            std::string input = gs.hud.chatContent.substr(1);

            CommandContext ctx{
                &gs.camera.camera,
                &gs.world.world,
                &gs.renderDistance,
                &gs.currentGamemode,
                &gs.input.isCreativeFlyEnabled
            };

            HandleCommand(input, ctx);
        } else if (!gs.hud.chatContent.empty()) {
            std::cout << "Message send: " << gs.hud.chatContent << std::endl;
        }

        gs.hud.chatContent.clear();
        gs.hud.isChatOpened = false;
        gs.input.isMovementsEnabled = true;
    }
}

static void handleDebugKeys(GameState& gs) {
    if (IsKeyPressed(KEY_F1)) {
        gs.hud.hideHUD = !gs.hud.hideHUD;
    }

    if (IsKeyPressed(KEY_F2)) {
        const std::filesystem::path outDir = std::filesystem::path(genPath("","screenshots/"));
        std::filesystem::create_directories(outDir);

        const auto now = std::chrono::system_clock::now();
        const std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm{};
        localtime_r(&t, &tm);

        std::ostringstream oss;
        oss << "screenshot_" << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << ".png";

        const std::filesystem::path filePath = outDir / oss.str();
        TakeScreenshot(filePath.string().c_str());

        std::cout << "Taken screenshot at " << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S") << std::endl;
    }

    if (IsKeyPressed(KEY_F3)) {
        if (IsKeyDown(KEY_B)) {
            gs.hud.drawBoundingBoxes = true;
            gs.hud.drawChunksGrid = false;
        } else if (IsKeyDown(KEY_G)) {
            gs.hud.drawChunksGrid = true;
            gs.hud.drawBoundingBoxes = false;
        } else {
            gs.hud.f3enabled = !gs.hud.f3enabled;
        }
    }

    if (IsKeyPressed(KEY_F5)) {
        gs.camera.cameraMode = (gs.camera.cameraMode == CAMERA_THIRD_PERSON)
            ? CAMERA_FIRST_PERSON
            : CAMERA_THIRD_PERSON;
    }
}

static void handleBlockKeys(GameState& gs) {
    if (IsKeyPressed(KEY_MINUS)) {
        gs.input.handedBlockId += 1;
    }
    if (IsKeyPressed(KEY_EQUAL)) {
        gs.input.handedBlockId -= 1;
    }

    if (IsKeyPressed(KEY_T)) {
        std::cout << "Chat opened\n";
        gs.hud.isChatOpened = true;
        gs.input.isMovementsEnabled = false;
        DisableCursor();
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) gs.input.queuedBreak = true;
    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) gs.input.queuedPlace = true;
    if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
        gs.input.handedBlockId = gs.input.currentHit.id;
    }
}

static void updateChunkBudget(GameState& gs) {
    int32_t camCx = (int32_t)std::floor(gs.camera.camera.position.x / (float)CHUNK_SIZE);
    int32_t camCy = (int32_t)std::floor(gs.camera.camera.position.y / (float)CHUNK_SIZE);
    int32_t camCz = (int32_t)std::floor(gs.camera.camera.position.z / (float)CHUNK_SIZE);

    for (int ox = -gs.renderDistance; ox <= gs.renderDistance; ++ox) {
        for (int oy = -gs.renderDistance; oy <= gs.renderDistance; ++oy) {
            for (int oz = -gs.renderDistance; oz <= gs.renderDistance; ++oz) {
                int cx = camCx + ox;
                int cy = camCy + oy;
                int cz = camCz + oz;

                if (!gs.world.world.HasChunkAt(cx, cy, cz)) {
                    gs.world.world.EnsureChunk(cx, cy, cz);
                    gs.world.world.MarkChunkAsDirty(cx, cy, cz);
                }

                auto chunk = gs.world.world.GetChunkAt(cx, cy, cz);
                if (!chunk) continue;

                if (!chunk->IsChunkLoaded() || chunk->IsModelEmpty()) {
                    gs.world.world.MarkChunkAsDirty(cx, cy, cz);
                }
            }
        }
    }

    gs.world.world.ProcessDirtyQueue((int32_t)MAX_DIRTY_CHUNKS_PER_FRAME, [&](int32_t cx, int32_t cy, int32_t cz) {
        auto chunk = gs.world.world.GetChunkAt(cx, cy, cz);
        if (!chunk || !chunk->IsChunkDirty()) return;

        chunk->SetState(ChunkState::Meshing);

        Model m = BuildModelForChunk(chunk, &gs.world.world);

        if (m.meshCount > 0) {
            chunk->UpdateChunkModel(m);
            if (gs.resources.atlas.id) {
                chunk->SetChunkMaterialTexture(gs.resources.atlas);
            }
            chunk->MarkAsLoaded();
            chunk->UnmarkAsDirty();
            chunk->SetState(ChunkState::Ready);
        } else {
            chunk->UpdateChunkModel(Model{0});
            chunk->MarkAsLoaded();
            chunk->UnmarkAsDirty();
            chunk->SetState(ChunkState::Generated);
        }
    });
}

static void updateMovementAndRaycast(GameState& gs) {
    float frame_dt = GetFrameTime();
    if (frame_dt > 0.25f) frame_dt = 0.25f;

    Vector2 mouseDelta = GetMouseDelta();
    gs.camera.zoom = GetMouseWheelMove() * 0.5f;

    gs.camera.rotation.x -= mouseDelta.x * gs.camera.sensitivityX;
    gs.camera.rotation.y -= mouseDelta.y * gs.camera.sensitivityY;

    if (gs.currentGamemode == BUILDER) {
        gs.input.applyBlockPlacementRestrictions = false;
        gs.input.reach = DEFAULT_REACH_BUILDER;
    } else {
        gs.input.applyBlockPlacementRestrictions = true;
        gs.input.reach = DEFAULT_REACH_SURVIVAL;
    }

    if (gs.currentGamemode == SPECTATOR) {
        gs.input.breakingAllowed = false;
        gs.input.placingAllowed = false;
    } else {
        gs.input.breakingAllowed = true;
        gs.input.placingAllowed = true;
    }

    UpdatePlayerMovementTick(
        gs.camera.camera,
        gs.camera.body,
        gs.camera.renderState,
        gs.world.world,
        gs.currentGamemode,
        gs.input.isCreativeFlyEnabled,
        gs.camera.movement,
        gs.camera.rotation,
        gs.input.accumulatorPlayer,
        gs.input.tickAccumulator,
        frame_dt,
        mouseDelta,
        gs.camera.zoom,
        gs.input.isMovementsEnabled,
        gs.input.isMouseEnabled
    );

    double now = GetTime();
    gs.input.accumulator += (now - gs.input.lastTime);
    gs.input.lastTime = now;

    while (gs.input.accumulator >= TICK_DT) {
        Ray ray = GetMouseRay(
            { (float)GetScreenWidth() * 0.5f, (float)GetScreenHeight() * 0.5f },
            gs.camera.camera
        );
        ray.direction = Vector3Normalize(ray.direction);

        gs.input.currentHit = UpdateRaycastingTick(
            ray,
            gs.camera.camera,
            gs.input.queuedBreak,
            gs.input.queuedPlace,
            gs.world.world,
            gs.input.handedBlockId,
            gs.input.breakingAllowed,
            gs.input.placingAllowed,
            gs.input.applyBlockPlacementRestrictions,
            gs.input.blockPlacingCooldown,
            gs.input.reach
        );

        gs.input.queuedBreak = false;
        gs.input.queuedPlace = false;
        gs.input.accumulator -= TICK_DT;
    }
}

void updateGame(GameState& gs) {
    handleWindowEvents(gs);
    if (gs.shouldExit) return;

    if (gs.currentScreen != GAME) {
        return;
    }

    handleEscape(gs);

    if (gs.input.isGamePaused) {
        return;
    }

    if (!IsCursorHidden() && !gs.input.isMouseEnabled) {
        DisableCursor();
    }

    updateChunkBudget(gs);
    handleChat(gs);
    handleDebugKeys(gs);
    handleBlockKeys(gs);
    updateMovementAndRaycast(gs);
}
