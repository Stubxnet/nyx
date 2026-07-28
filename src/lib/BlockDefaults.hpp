#pragma once

#include <raylib.h>
#include <cstdint>
#include <string>
#include <array>
#include <vector>
#include <algorithm>
#include <optional>
#include <unordered_map>
#include "../enum.hpp"
#include "../lib/BlockData.hpp"

constexpr int TILE_SIZE = 32;

enum Face : int {
    TOP = 0,    // +Y
    BOTTOM,     // -Y
    EAST,       // +X
    WEST,       // -X
    NORTH,      // -Z
    SOUTH       // +Z
};

static const char* FaceNames[6] = {
    "top","bottom","east","west","north","south"
};

struct FaceTexture {
    std::string textureName;
    Rectangle uv{};
};

struct BlockReference {
    uint16_t id = 0;
    std::string displayName;
    uint16_t resistance = 0;
    BlockType type = BlockType::FULL;
    BlockMaterial material = BlockMaterial::DIRT;
    std::array<FaceTexture, 6> faces;
    std::optional<BlockExtra> extra;
};

struct LoadedBlockDefaults {
    std::unordered_map<uint16_t, BlockReference> blocks;
    std::optional<BlockReference> defaultBlock;
};

struct BlocksDefaults {
    LoadedBlockDefaults loaded;
    std::vector<uint16_t> sortedIds;
    size_t currentIndex = 0;

    RenderTexture2D atlasRT{};
    Texture2D atlasTex{};
    bool atlasReady = false;
    int atlasSideTiles = 0;
    uint32_t atlasW = 0;
    uint32_t atlasH = 0;
    uint16_t atlasCols = 0;
    uint16_t atlasRows = 0;

    std::vector<Texture2D> atlasTextures;
};

static inline void sortIds(BlocksDefaults& blocksDefaults) {
    blocksDefaults.sortedIds.clear();
    blocksDefaults.sortedIds.reserve(blocksDefaults.loaded.blocks.size());
    for (auto& [id, _] : blocksDefaults.loaded.blocks)
        blocksDefaults.sortedIds.push_back(id);
    std::sort(blocksDefaults.sortedIds.begin(), blocksDefaults.sortedIds.end());
}

static inline void unloadAtlas(BlocksDefaults& blocksDefaults) {
    for (auto& t : blocksDefaults.atlasTextures) UnloadTexture(t);
    blocksDefaults.atlasTextures.clear();

    if (blocksDefaults.atlasReady) {
        if (blocksDefaults.atlasRT.id != 0) UnloadRenderTexture(blocksDefaults.atlasRT);
        if (blocksDefaults.atlasTex.id != 0) UnloadTexture(blocksDefaults.atlasTex);
        blocksDefaults.atlasTex = Texture2D{};
        blocksDefaults.atlasReady = false;
    }
}
