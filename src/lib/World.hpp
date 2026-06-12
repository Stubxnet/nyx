#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <functional>
#include "Chunk.hpp"
#include "../enum.hpp"

static inline int64_t ChunkKey(int cx, int cy, int cz) {
    const int64_t MASK = (1LL << 21) - 1;
    auto enc = ( (int64_t)(cx & MASK) << 42 ) | ( (int64_t)(cy & MASK) << 21 ) | (int64_t)(cz & MASK);
    return enc;
}

class World {
public:
    World(const std::string& name, Vector3 spawnpoint) : name(name), spawnpoint(spawnpoint) {}

    using ChunkModifiedCallback = std::function<void(int32_t cx,int32_t cy,int32_t cz)>;
    void SetChunkModifiedCallback(ChunkModifiedCallback cb) { chunkModifiedCb = cb; }

    bool IsChunkEmpty(int32_t cx, int32_t cy, int32_t cz) const {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return true;
        return chunk->IsChunkEmpty();
    }

    bool IsChunkDirty(int32_t cx, int32_t cy, int32_t cz) const {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return false;
        return chunk->IsChunkDirty();
    }

    void MarkChunkAsDirty(int32_t cx, int32_t cy, int32_t cz) {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->MarkAsDirty();
    }

    void UnmarkChunkAsDirty(int32_t cx, int32_t cy, int32_t cz) {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->UnmarkAsDirty();
    }

    bool IsChunkLoaded(int32_t cx, int32_t cy, int32_t cz) const {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return false;
        return chunk->IsChunkLoaded();
    }

    void MarkChunkAsLoaded(int32_t cx, int32_t cy, int32_t cz) {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->MarkAsLoaded();
    }

    void UnmarkChunkAsLoaded(int32_t cx, int32_t cy, int32_t cz) {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->UnmarkAsLoaded();
    }

    bool IsModelEmpty(int32_t cx, int32_t cy, int32_t cz) const {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return true;
        return chunk->IsModelEmpty();
    }

    void UnloadChunk(int32_t cx, int32_t cy, int32_t cz) {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->UnloadChunk();
    }

    void UpdateChunkModel(int32_t cx, int32_t cy, int32_t cz, Model& model) {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->UpdateChunkModel(model);
    }

    void SetChunkMaterialTexture(int32_t cx, int32_t cy, int32_t cz, Texture2D& atlas) {
        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return;
        chunk->SetChunkMaterialTexture(atlas);
    }

    void Rendered(int32_t cx, int32_t cy, int32_t cz) {
        renderedChunks.push_back(ChunkKey(cx, cy, cz));
    }

    bool IsRendered(int32_t cx, int32_t cy, int32_t cz) const {
        int64_t key = ChunkKey(cx, cy, cz);
        return std::find(renderedChunks.begin(), renderedChunks.end(), key) != renderedChunks.end();
    }

    // world block coordinates -> id (missing chunk/block => air => 0)
    int GetBlockId(int64_t worldX, int64_t worldY, int64_t worldZ) const {
        auto [cx, lx] = WorldToChunkAndLocal((int)worldX);
        auto [cy, ly] = WorldToChunkAndLocal((int)worldY);
        auto [cz, lz] = WorldToChunkAndLocal((int)worldZ);

        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return 0;
        auto block = chunk->GetBlock(lx, ly, lz);
        if (!block) return 0;
        return block->GetId();
    }

    bool SetBlock(int64_t worldX, int64_t worldY, int64_t worldZ, int id, SetblockActions action = SetblockActions::SET) {
        auto [cx, lx] = WorldToChunkAndLocal((int)worldX);
        auto [cy, ly] = WorldToChunkAndLocal((int)worldY);
        auto [cz, lz] = WorldToChunkAndLocal((int)worldZ);

        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return false;

        int current = 0;
        auto block = chunk->GetBlock(lx, ly, lz);
        if (block) current = block->GetId();

        auto notifyNeighbors = [&](void) {
            if (!chunkModifiedCb) return;
            chunkModifiedCb(cx, cy, cz);
            if (lx == 0) chunkModifiedCb(cx-1, cy, cz);
            if (lx == CHUNK_SIZE-1) chunkModifiedCb(cx+1, cy, cz);
            if (ly == 0) chunkModifiedCb(cx, cy-1, cz);
            if (ly == CHUNK_SIZE-1) chunkModifiedCb(cx, cy+1, cz);
            if (lz == 0) chunkModifiedCb(cx, cy, cz-1);
            if (lz == CHUNK_SIZE-1) chunkModifiedCb(cx, cy, cz+1);
        };

        switch (action) {
            case SetblockActions::SET:
                chunk->SetBlockId(lx, ly, lz, id);
                notifyNeighbors();
                return true;
            case SetblockActions::REPLACE:
                if (current != 0) {
                    chunk->SetBlockId(lx, ly, lz, id);
                    notifyNeighbors();
                    return true;
                }
                return false;
            case SetblockActions::KEEP:
                if (current == 0) {
                    chunk->SetBlockId(lx, ly, lz, id);
                    notifyNeighbors();
                    return true;
                }
                return false;
            case SetblockActions::BREAK:
                if (current != 0) {
                    chunk->SetBlockId(lx, ly, lz, 0);
                    notifyNeighbors();
                    return true;
                }
                return false;
        }
        return false;
    }

    size_t GetChunkCount() const { return chunks.size(); }

    std::shared_ptr<Chunk> GetChunk(size_t index) const {
        if (index < chunkList.size()) return chunkList[index];
        return nullptr;
    }

    std::shared_ptr<Chunk> GetChunkAt(int32_t cx, int32_t cy, int32_t cz) const {
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

    int64_t FillBlocks(int64_t ax, int64_t ay, int64_t az,
                    int64_t bx, int64_t by, int64_t bz,
                    const BlockFillActions& blockaction,
                    int id = 1,
                    int64_t blockLimit = 50000)
    {
        int64_t x0 = std::min(ax, bx), x1 = std::max(ax, bx);
        int64_t y0 = std::min(ay, by), y1 = std::max(ay, by);
        int64_t z0 = std::min(az, bz), z1 = std::max(az, bz);

        __int128 dx = (__int128)x1 - (__int128)x0 + 1;
        __int128 dy = (__int128)y1 - (__int128)y0 + 1;
        __int128 dz = (__int128)z1 - (__int128)z0 + 1;
        __int128 volume = dx * dy * dz;
        if (volume <= 0) return 0;
        if (volume > blockLimit) return 0;

        int64_t placed = 0;

        auto isOutline = [&](int64_t x, int64_t y, int64_t z)->bool{
            return (x == x0 || x == x1) || (y == y0 || y == y1) || (z == z0 || z == z1);
        };

        for (int64_t x = x0; x <= x1; ++x) {
            for (int64_t y = y0; y <= y1; ++y) {
                for (int64_t z = z0; z <= z1; ++z) {
                    bool shouldProcess = true;
                    if (blockaction == BlockFillActions::OUTLINE) {
                        shouldProcess = isOutline(x,y,z);
                    }

                    if (!shouldProcess) continue;

                    int current = GetBlockId(x, y, z);

                    switch (blockaction) {
                        case BlockFillActions::SET:
                            if (SetBlock(x, y, z, id, SetblockActions::SET))
                                ++placed;
                            break;
                        case BlockFillActions::REPLACE:
                            if (current != 0) {
                                if (SetBlock(x, y, z, id, SetblockActions::SET))
                                    ++placed;
                            }
                            break;
                        case BlockFillActions::KEEP:
                            if (current == 0) {
                                if (SetBlock(x, y, z, id, SetblockActions::SET))
                                    ++placed;
                            }
                            break;
                        case BlockFillActions::BREAK:
                            if (current != 0) {
                                if (SetBlock(x, y, z, 0, SetblockActions::SET))
                                    ++placed;
                            }
                            break;
                        case BlockFillActions::OUTLINE:
                            if (SetBlock(x, y, z, id, SetblockActions::SET))
                                ++placed;
                            break;
                    }

                    if (placed >= blockLimit) return placed;
                }
            }
        }

        return placed;
    }

    Vector3 GetSpawnPoint(void) const { return spawnpoint; }
    void SetSpawnPoint(Vector3& newSpawnpoint) { spawnpoint = newSpawnpoint; }

private:
    std::unordered_map<int64_t, std::shared_ptr<Chunk>> chunks;
    std::vector<std::shared_ptr<Chunk>> chunkList; // optional list
    std::string name;
    std::vector<int64_t> renderedChunks;
    std::vector<int64_t> loadedChunks;
    Vector3 spawnpoint;
    ChunkModifiedCallback chunkModifiedCb;
};
