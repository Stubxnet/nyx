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

    gs.hud.fps = GetFPS();
    if (gs.hud.fps < 10) gs.hud.fpsColor = RED;
    else if (gs.hud.fps < 30) gs.hud.fpsColor = ORANGE;
    else gs.hud.fpsColor = GREEN;

    DrawText(TextFormat("FPS: %i (Target: %i)", GetFPS(), gs.targetFPS), 15, gs.hud.textSpacing, gs.hud.textSize, gs.hud.fpsColor);
    DrawText("Nyx build pre-release 1.0.0", 15, gs.hud.textSpacing + gs.hud.lineSize, gs.hud.textSize, gs.hud.f3color);
    DrawText("Camera controls:", 15, gs.hud.textSpacing + gs.hud.lineSize*2, gs.hud.textSize, gs.hud.f3color);
    DrawText("W, A, S, D, Space, Shift to move", 15, gs.hud.textSpacing + gs.hud.lineSize*3, gs.hud.textSize, gs.hud.f3color);
    DrawText("Arrow keys or mouse to look around", 15, gs.hud.textSpacing + gs.hud.lineSize*4, gs.hud.textSize, gs.hud.f3color);
    DrawText("T to open chat", 15, gs.hud.textSpacing + gs.hud.lineSize*5, gs.hud.textSize, gs.hud.f3color);
    DrawText("Zoom keys: num-plus, num-minus or mouse scroll", 15, gs.hud.textSpacing + gs.hud.lineSize*6, gs.hud.textSize, gs.hud.f3color);

    DrawText("Current camera status:", 15, gs.hud.textSpacing + gs.hud.lineSize*7, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Camera mode: %s", (gs.camera.cameraMode == CAMERA_FREE) ? "FREE" :
        (gs.camera.cameraMode == CAMERA_FIRST_PERSON) ? "FIRST_PERSON" :
        (gs.camera.cameraMode == CAMERA_THIRD_PERSON) ? "THIRD_PERSON" :
        (gs.camera.cameraMode == CAMERA_ORBITAL) ? "ORBITAL" : "CUSTOM"), 15, gs.hud.textSpacing + gs.hud.lineSize*8, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Projection: %s", (gs.camera.camera.projection == CAMERA_PERSPECTIVE) ? "PERSPECTIVE" :
        (gs.camera.camera.projection == CAMERA_ORTHOGRAPHIC) ? "ORTHOGRAPHIC" : "CUSTOM"), 15, gs.hud.textSpacing + gs.hud.lineSize*9, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Position: (%06.3f, %06.3f, %06.3f)",
        gs.camera.camera.position.x, gs.camera.camera.position.y, gs.camera.camera.position.z), 15, gs.hud.textSpacing + gs.hud.lineSize*10, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Target: (%06.3f, %06.3f, %06.3f)",
        gs.camera.camera.target.x, gs.camera.camera.target.y, gs.camera.camera.target.z), 15, gs.hud.textSpacing + gs.hud.lineSize*11, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Up: (%06.3f, %06.3f, %06.3f)",
        gs.camera.camera.up.x, gs.camera.camera.up.y, gs.camera.camera.up.z), 15, gs.hud.textSpacing + gs.hud.lineSize*12, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Chunks on world: %d", (int)gs.world.world.GetChunkCount()), 15, gs.hud.textSpacing + gs.hud.lineSize*13, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Blocks on world (including air): %d", (int)gs.world.world.GetChunkCount()*16), 15, gs.hud.textSpacing + gs.hud.lineSize*14, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Render distance: %d", gs.renderDistance), 15, gs.hud.textSpacing + gs.hud.lineSize*15, gs.hud.textSize, gs.hud.f3color);
    DrawText(TextFormat("Gamemode: %s",
        (gs.currentGamemode == SURVIVAL) ? "SURVIVAL" :
        (gs.currentGamemode == CREATIVE) ? "CREATIVE" :
        (gs.currentGamemode == SPECTATOR) ? "SPECTATOR" :
        (gs.currentGamemode == BUILDER) ? "BUILDER" : "UNKNOWN"
    ), 15, gs.hud.textSpacing + gs.hud.lineSize*16, gs.hud.textSize, gs.hud.f3color);
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
