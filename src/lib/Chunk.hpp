#pragma once
#include "../constants.hpp"
#include <memory>
#include <array>
#include <cstdint>
#include <cmath>
#include "Block.hpp"
#include "raylib.h"

struct ChunkKey { int32_t x, y, z; };

class Chunk {
public:
    Chunk(int cx = 0, int cy = 0, int cz = 0)
        : cx(cx), cy(cy), cz(cz),
          empty(true), defaultid(0),
          dirty(false), loaded(false),
          model{0}
    {
        InitializeBlocks(defaultid);
    }

    std::shared_ptr<Block> GetBlock(int x, int y, int z) const {
        if (IsValidLocalPosition(x, y, z)) return blocks[x][y][z];
        return nullptr;
    }

    void SetBlockId(int x, int y, int z, int newId) {
        if (IsValidLocalPosition(x, y, z) && blocks[x][y][z]) {
            blocks[x][y][z]->SetId(newId);
            if (newId != 0) empty = false;
        }
    }

    int GetChunkX() const { return cx; }
    int GetChunkY() const { return cy; }
    int GetChunkZ() const { return cz; }

    bool IsChunkEmpty() const { return empty; }

    bool IsChunkDirty() const { return dirty; }
    void MarkAsDirty() { dirty = true; }
    void UnmarkAsDirty() { dirty = false; }

    bool IsChunkLoaded() const { return loaded; }
    void MarkAsLoaded() { loaded = true; }
    void UnmarkAsLoaded() { loaded = false; }

    Model& GetModel() { return model; }

    bool IsModelEmpty() {
        return (model.meshCount == 0);
    }

    void UpdateChunkModel(Model& m) { model = m; }

    void SetChunkMaterialTexture(Texture2D& atlas) {
        SetMaterialTexture(&model.materials[0], MATERIAL_MAP_DIFFUSE, atlas);
    }

    void UnloadChunk() {
        UnloadModel(model);
        model = Model{0};
        loaded = false;
        dirty = false;
    }


private:
    std::array<std::array<std::array<std::shared_ptr<Block>, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> blocks;
    int cx, cy, cz;
    bool empty;
    int defaultid;
    bool dirty;
    bool loaded;
    Model model{0};

    void InitializeBlocks(int id) {
        for (int x = 0; x < CHUNK_SIZE; ++x)
            for (int y = 0; y < CHUNK_SIZE; ++y)
                for (int z = 0; z < CHUNK_SIZE; ++z)
                    blocks[x][y][z] = std::make_shared<Block>(id);
        empty = (id == 0);
    }

    bool IsValidLocalPosition(int x, int y, int z) const {
        return (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE);
    }
};
