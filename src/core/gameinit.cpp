#include "gamestate.hpp"
#include "../lib/Config.hpp"

#include <iostream>

static void initWindowAndRender(GameState& gs, const Config& config) {
    InitWindow(config.windowWidth, config.windowHeight, config.windowTitle.c_str());
    SetExitKey(KEY_NULL);
    SetTargetFPS(gs.targetFPS);

    gs.window.width = GetRenderWidth();
    gs.window.height = GetRenderHeight();
}

static void loadResources(GameState& gs, const Config& config) {
    gs.resources.background = LoadTexture(
        genPath(config.gameDirectory, "assets/textures/background/background1920x1080p.png").c_str()
    );

    const fs::path gameDir = fs::path(config.gameDirectory);
    const fs::path texturesDir = gameDir / "assets/blocks/textures";
    const fs::path savesDir    = gameDir / "assets/blocks/saves";
    const fs::path atlasPngPath = savesDir / "atlas.png";
    const fs::path atlasUvPath  = savesDir / "atlas_uv.json";

    auto loaded = loadBlockReferences(genPath(config.gameDirectory, "assets/blocks/references/"), texturesDir);
    gs.resources.blocksDefaults.loaded = std::move(loaded);

    sortIds(gs.resources.blocksDefaults);

    if (!config.atlasRegeneration && tryLoadSavedAtlas(gs.resources.blocksDefaults, atlasPngPath, atlasUvPath)) {
        std::cout << "Loaded saved atlas from disk: " << atlasPngPath.string() << "\n";
    } else {
        buildAtlasForBlocks(gs.resources.blocksDefaults, texturesDir, atlasPngPath, atlasUvPath);
    }

    gs.resources.atlas = gs.resources.blocksDefaults.atlasTex;
    SetAtlasTexture(gs.resources.atlas);
    SetBlockDefaults(&gs.resources.blocksDefaults);
}

static void initWorld(GameState& gs) {
    gs.world = WorldState("Default World", {0.0f, 9.5f, 2.0f});

    auto& world = gs.world.world;

    int range = gs.renderDistance;
    for (int cx = -range; cx <= range; ++cx) {
        for (int cy = -range; cy <= range; ++cy) {
            for (int cz = -range; cz <= range; ++cz) {
                world.AddChunk(std::make_shared<Chunk>(cx, cy, cz));
            }
        }
    }

    world.MarkAllChunksDirty();

    world.FillBlocks(16, -2, 16, -16, -16, -16, BlockFillActions::SET, 4);
    world.FillBlocks(16, -1, 16, -16, -1, -16, BlockFillActions::SET, 2);
    world.FillBlocks(16, 0, 16, -16, 0, -16, BlockFillActions::SET, 1);
    world.FillBlocks(2, 0, 2, 2, 10, 14, BlockFillActions::SET, 9);
}

static void initCamera(GameState& gs) {
    Vector3 spawn = gs.world.world.GetSpawnPoint();

    gs.camera.camera = {};
    gs.camera.camera.position = spawn;
    gs.camera.camera.target = { 0.0f, 2.0f, 0.0f };
    gs.camera.camera.up = { 0.0f, 1.0f, 0.0f };
    gs.camera.camera.fovy = 60.0f;
    gs.camera.camera.projection = CAMERA_PERSPECTIVE;

    gs.camera.renderState.currentCameraPosition = gs.camera.camera.position;
    gs.camera.renderState.previousCameraPosition = gs.camera.camera.position;

    gs.camera.cameraMode = CAMERA_FIRST_PERSON;
    gs.camera.rotation = { DEFAULT_ROTATION_X, DEFAULT_ROTATION_Y, DEFAULT_ROTATION_Z };
    gs.camera.movement = { DEFAULT_MOVEMENT_X, DEFAULT_MOVEMENT_Y, DEFAULT_MOVEMENT_Z };
    gs.camera.zoom = DEFAULT_ZOOM;
}

std::unique_ptr<GameState> initGame(const Config& config) {
    auto gs = std::make_unique<GameState>();

    initWindowAndRender(*gs, config);
    loadResources(*gs, config);
    initWorld(*gs);
    initCamera(*gs);

    gs->input.lastTime = GetTime();

    gs->hud.textSize = GetRenderHeight() / 50;
    gs->hud.textSpacing = gs->hud.textSize / 2;
    gs->hud.lineSize = gs->hud.textSpacing + gs->hud.textSize;
    gs->hud.chatTextSize = GetRenderHeight() / 50;

    return gs;
}
