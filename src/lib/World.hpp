#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include "Chunk.hpp"

static inline int64_t ChunkKey(int cx, int cy, int cz) {
    const int64_t MASK = (1LL << 21) - 1;
    auto enc = ( (int64_t)(cx & MASK) << 42 ) | ( (int64_t)(cy & MASK) << 21 ) | (int64_t)(cz & MASK);
    return enc;
}

class World {
public:
    World(const std::string& name, Vector3 spawnpoint) : name(name), spawnpoint(spawnpoint) {}

    // world block coordinates -> id (missing chunk/block => air => 0)
    int GetBlockId(int worldX, int worldY, int worldZ) const {
        auto [cx, lx] = WorldToChunkAndLocal(worldX);
        auto [cy, ly] = WorldToChunkAndLocal(worldY);
        auto [cz, lz] = WorldToChunkAndLocal(worldZ);

        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return 0;
        auto block = chunk->GetBlock(lx, ly, lz);
        if (!block) return 0;
        return block->GetId();
    }

    void SetBlockId(int worldX, int worldY, int worldZ, int id) {
        auto [cx, lx] = WorldToChunkAndLocal(worldX);
        auto [cy, ly] = WorldToChunkAndLocal(worldY);
        auto [cz, lz] = WorldToChunkAndLocal(worldZ);

        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->SetBlockId(lx, ly, lz, id);
    }

    size_t GetChunkCount() const { return chunks.size(); }

    std::shared_ptr<Chunk> GetChunk(size_t index) const {
        if (index < chunkList.size()) return chunkList[index];
        return nullptr;
    }

    std::shared_ptr<Chunk> GetChunkAt(int cx, int cy, int cz) const {
        int64_t key = ChunkKey(cx, cy, cz);
        auto it = chunks.find(key);
        if (it == chunks.end()) return nullptr;
        return it->second;
    }

    bool IsBlockTransparent(int worldX, int worldY, int worldZ) const {
        return GetBlockId(worldX, worldY, worldZ) == 0;
    }

    void AddChunk(std::shared_ptr<Chunk> chunk) {
        int64_t key = ChunkKey(chunk->GetChunkX(), chunk->GetChunkY(), chunk->GetChunkZ());
        chunks[key] = chunk;
        chunkList.push_back(chunk);
    }

    static std::pair<int,int> WorldToChunkAndLocal(int w) {
        int c = (w >= 0) ? (w / CHUNK_SIZE) : ((w + 1) / CHUNK_SIZE - 1);
        int local = w - c * CHUNK_SIZE;
        return {c, local};
    }

private:
    std::unordered_map<int64_t, std::shared_ptr<Chunk>> chunks;
    std::vector<std::shared_ptr<Chunk>> chunkList; // optional list
    std::string name;
    Vector3 spawnpoint;
};
