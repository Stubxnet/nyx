#pragma once
#include "../constants.hpp"
#include <array>
#include <cstdint>
#include "raylib.h"

class Chunk {
public:
    using BlockId = uint16_t;

    Chunk(int32_t cx = 0, int32_t cy = 0, int32_t cz = 0)
        : cx(cx), cy(cy), cz(cz),
          empty(true), dirty(false), loaded(false),
          state(ChunkState::Unloaded), model{0}
    {
        InitializeBlocks(0);
    }

    ~Chunk() {
        if (model.meshCount > 0) {
            UnloadModel(model);
            model = Model{0};
        }
    }

    int32_t GetChunkX() const { return cx; }
    int32_t GetChunkY() const { return cy; }
    int32_t GetChunkZ() const { return cz; }

    ChunkState GetState() const { return state; }
    void SetState(ChunkState s) { state = s; }

    BlockId GetBlock(int x, int y, int z) const {
        if (!IsValidLocalPosition(x, y, z)) return 0;
        return blocks[x][y][z];
    }

    void SetBlockId(int x, int y, int z, BlockId newId) {
        if (!IsValidLocalPosition(x, y, z)) return;
        blocks[x][y][z] = newId;
        dirty = true;
        if (newId != 0) empty = false;
    }

    bool IsChunkEmpty() const { return empty; }
    bool IsChunkDirty() const { return dirty; }
    bool IsChunkLoaded() const { return loaded; }

    void MarkAsDirty() { dirty = true; state = ChunkState::Dirty; }
    void UnmarkAsDirty() { dirty = false; }

    void MarkAsLoaded() { loaded = true; state = ChunkState::Ready; }
    void UnmarkAsLoaded() { loaded = false; }

    Model& GetModel() { return model; }

    bool IsModelEmpty() const { return model.meshCount == 0; }

    void UpdateChunkModel(Model m) {
        if (model.meshCount > 0) {
            UnloadModel(model);
        }
        model = m;
    }

    void SetChunkMaterialTexture(Texture2D& atlas) {
        if (model.meshCount > 0 && model.materials) {
            SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, atlas);
        }
    }

    void UnloadChunk() {
        if (model.meshCount > 0) {
            UnloadModel(model);
            model = Model{0};
        }
        loaded = false;
        dirty = false;
        state = ChunkState::Unloaded;
    }

private:
    std::array<std::array<std::array<BlockId, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> blocks;
    int32_t cx, cy, cz;
    bool empty;
    bool dirty;
    bool loaded;
    ChunkState state;
    Model model{0};

    void InitializeBlocks(BlockId id) {
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                for (int z = 0; z < CHUNK_SIZE; ++z)
                    blocks[x][y][z] = id;
        empty = (id == 0);
    }

    bool IsValidLocalPosition(int x, int y, int z) const {
        return x >= 0 && x < CHUNK_SIZE &&
               y >= 0 && y < CHUNK_SIZE &&
               z >= 0 && z < CHUNK_SIZE;
    }
};
