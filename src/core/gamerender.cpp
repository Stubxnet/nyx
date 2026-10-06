#include "gamestate.hpp"

static void drawCrosshair() {
    int centerx = GetScreenWidth() / 2;
    int centery = GetScreenHeight() / 2;

    DrawLine(centerx - 10, centery, centerx - 3, centery, WHITE);
    DrawLine(centerx + 3,  centery, centerx + 10, centery, WHITE);
    DrawLine(centerx, centery - 10, centerx, centery - 3, WHITE);
    DrawLine(centerx, centery + 3,  centerx, centery + 10, WHITE);
}

static void drawWorld(GameState& gs) {
    int32_t camCx = (int32_t)std::floor(gs.camera.camera.position.x / (float)CHUNK_SIZE);
    int32_t camCy = (int32_t)std::floor(gs.camera.camera.position.y / (float)CHUNK_SIZE);
    int32_t camCz = (int32_t)std::floor(gs.camera.camera.position.z / (float)CHUNK_SIZE);

    gs.world.world.ClearRendered();

    for (int ox = -gs.renderDistance; ox <= gs.renderDistance; ++ox) {
        for (int oy = -gs.renderDistance; oy <= gs.renderDistance; ++oy) {
            for (int oz = -gs.renderDistance; oz <= gs.renderDistance; ++oz) {
                int cx = camCx + ox;
                int cy = camCy + oy;
                int cz = camCz + oz;

                auto chunk = gs.world.world.GetChunkAt(cx, cy, cz);
                if (!chunk || !chunk->IsChunkLoaded() || chunk->IsModelEmpty()) continue;

                float worldX = (float)(cx * CHUNK_SIZE);
                float worldY = (float)(cy * CHUNK_SIZE);
                float worldZ = (float)(cz * CHUNK_SIZE);

                DrawModel(chunk->GetModel(), { worldX, worldY, worldZ }, 1.0f, WHITE);
                gs.world.world.Rendered(cx, cy, cz);
            }
        }
    }

    if (gs.hud.drawBoundingBoxes && gs.camera.camera.projection == CAMERA_THIRD_PERSON) {
        BoundingBox playerBox = CreatePlayerHitbox(gs.camera.camera);
        DrawBoundingBox(playerBox, LIME);
    }

    if (gs.input.currentHit.hit) {
        Vector3 p = {
            (float)gs.input.currentHit.x + 0.5f,
            (float)gs.input.currentHit.y + 0.5f,
            (float)gs.input.currentHit.z + 0.5f
        };
        DrawCubeWires(p, 1.001f, 1.001f, 1.001f, (Color){ 220, 40, 40, 255 });
    }
}

static void drawF3(GameState& gs) {
    if (!gs.hud.f3enabled || gs.hud.hideHUD) return;

    const auto& p = gs.camera.camera.position;
    const int bx = (int)std::floor(p.x);
    const int by = (int)std::floor(p.y);
    const int bz = (int)std::floor(p.z);

    auto [cx, lx] = World::WorldToChunkAndLocal(bx);
    auto [cy, ly] = World::WorldToChunkAndLocal(by);
    auto [cz, lz] = World::WorldToChunkAndLocal(bz);

    auto chunk = gs.world.world.GetVerticalChunkAt(cx, cz);

    int surfaceY = 0;
    float temperature = 0.f, moisture = 0.f;
    float erosion = 0.f, continentalness = 0.f;
    const char* biomeName = "Unknown";

    if (chunk) {
        auto& cell = chunk->GetCell(lx, lz);
        surfaceY = cell.surfaceY;
        temperature = cell.temperature;
        moisture = cell.moisture;
        erosion = cell.erosion;
        continentalness = cell.continentalness;

        biomeName = GetBiomeName(cell.biome.id);
    }

    gs.hud.fps = GetFPS();
    gs.hud.fpsColor = gs.hud.fps < 10 ? RED :
                      gs.hud.fps < 30 ? ORANGE : GREEN;

    const int lineSize = gs.hud.lineSize;
    const int textSize = gs.hud.textSize;
    int yPos = gs.hud.textSpacing + lineSize;
    const Color color = gs.hud.f3color;

    auto drawLine = [&](const char* text) {
        DrawText(text, 15, yPos, textSize, color);
        yPos += lineSize;
    };

    DrawText(TextFormat("FPS: %i (Target: %i)", GetFPS(), gs.targetFPS),
             15, gs.hud.textSpacing, textSize, gs.hud.fpsColor);

    drawLine("Nyx build pre-release 1.0.0");
    drawLine("Current camera status:");

    const char* cameraModes[] = {
        "FREE", "FIRST_PERSON", "THIRD_PERSON", "ORBITAL", "CUSTOM"
    };
    drawLine(TextFormat("Camera mode: %s",
        cameraModes[std::min((int)gs.camera.cameraMode, 4)]));

    const char* projections[] = {
        "PERSPECTIVE", "ORTHOGRAPHIC", "CUSTOM"
    };
    drawLine(TextFormat("Projection: %s", projections[std::min((int)gs.camera.camera.projection, 2)]));

    drawLine(TextFormat("Position blocks: (%d, %d, %d)", bx, by, bz));
    drawLine(TextFormat("Chunk courant: (%d, %d) | local: (%d, %d)",
                         cx, cz, lx, lz));

    drawLine(TextFormat("Target: (%06.3f, %06.3f, %06.3f)",
        gs.camera.camera.target.x,
        gs.camera.camera.target.y,
        gs.camera.camera.target.z));

    drawLine(TextFormat("Chunks on world: %d",
        (int)gs.world.world.GetChunkCount()));
    drawLine(TextFormat("World seed: %d", (int32_t)gs.world.seed));
    drawLine(TextFormat("Render distance: %d", gs.renderDistance));
    drawLine(TextFormat("Surface Y: %d", surfaceY));
    drawLine(TextFormat("Temperature: %.3f", temperature));
    drawLine(TextFormat("Moisture: %.3f", moisture));
    drawLine(TextFormat("Erosion: %.3f", erosion));
    drawLine(TextFormat("Continentalness: %.3f", continentalness));
    drawLine(TextFormat("Biome: %s", biomeName));

    const char* gamemodes[] = {
        "SURVIVAL", "CREATIVE", "SPECTATOR", "BUILDER", "UNKNOWN"
    };
    drawLine(TextFormat("Gamemode: %s",
        gamemodes[std::min((int)gs.currentGamemode, 4)]));
}

static void drawChat(GameState& gs) {
    if (gs.hud.isChatOpened && !gs.hud.hideHUD) {
        DrawChat(gs.window.height, gs.window.width, gs.hud.chatTextSize, gs.hud.chatContent);
    }
}

static void drawPause(GameState& gs) {
    if (gs.input.isGamePaused) {
        DrawPauseScreen(gs.window.height, gs.window.width);
    }
}

void drawGame(GameState& gs) {
    if (gs.currentScreen != GAME) return;

    BeginDrawing();
    ClearBackground(backgroundColor);

    BeginMode3D(gs.camera.camera);
    drawWorld(gs);
    EndMode3D();

    drawCrosshair();
    drawF3(gs);
    drawChat(gs);
    drawPause(gs);

    EndDrawing();
}

void drawMenu(GameState& gs) {
    MenuAction action = DrawAndHandleMenu(gs.window.height, gs.window.width, gs.resources.background, backgroundColor, backgroundColor);
    if (action == MenuAction::PLAY) gs.currentScreen = GAME;
    else if (action == MenuAction::OPTIONS) gs.currentScreen = OPTIONS;
    else if (action == MenuAction::QUIT) gs.shouldExit = true;
}

void drawOptions(GameState& gs) {
    OptionsAction action = DrawAndHandleOptions(gs.window.width, gs.window.height, gs.resources.background, backgroundColor, backgroundColor);
    if (action == OptionsAction::QUIT) gs.shouldExit = true;
    else if (action == OptionsAction::BACK) gs.currentScreen = MENU;
}
