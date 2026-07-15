#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <cstdint>
#include <algorithm>
#include <functional>
#include <tuple>
#include <iostream>
#include "Chunk.hpp"

static inline int64_t PackChunkKey(int32_t x, int32_t y, int32_t z) {
    const int64_t BIAS = 1 << 20;
    const int64_t MASK = (1LL << 21) - 1;
    int64_t ux = ((int64_t)x + BIAS) & MASK;
    int64_t uy = ((int64_t)y + BIAS) & MASK;
    int64_t uz = ((int64_t)z + BIAS) & MASK;
    return (ux << 42) | (uy << 21) | uz;
}

static inline std::tuple<int32_t,int32_t,int32_t> UnpackChunkKey(int64_t key) {
    const int64_t BIAS = 1 << 20;
    const int64_t MASK = (1LL << 21) - 1;
    int32_t x = (int32_t)(((key >> 42) & MASK) - BIAS);
    int32_t y = (int32_t)(((key >> 21) & MASK) - BIAS);
    int32_t z = (int32_t)((key & MASK) - BIAS);
    return {x, y, z};
}

class World {
public:
    using ChunkModifiedCallback = std::function<void(int32_t, int32_t, int32_t)>;

    World(const std::string& name, Vector3 spawnpoint) : name(name), spawnpoint(spawnpoint) {}

    ~World() {
        ClearAllChunks();
    }

    void SetChunkModifiedCallback(ChunkModifiedCallback cb) { chunkModifiedCb = cb; }

    std::shared_ptr<Chunk> GetChunkAt(int32_t cx, int32_t cy, int32_t cz) const {
        auto it = chunks.find(PackChunkKey(cx, cy, cz));
        if (it == chunks.end()) return nullptr;
        return it->second;
    }

    void AddChunk(const std::shared_ptr<Chunk>& chunk) {
        int64_t key = PackChunkKey(chunk->GetChunkX(), chunk->GetChunkY(), chunk->GetChunkZ());
        chunks[key] = chunk;
    }

    void RemoveChunk(int32_t cx, int32_t cy, int32_t cz) {
        int64_t key = PackChunkKey(cx, cy, cz);
        auto it = chunks.find(key);
        if (it == chunks.end()) return;

        if (it->second) {
            it->second->SetState(ChunkState::Unloading);
            it->second->UnloadChunk();
        }

        chunks.erase(it);
        renderedChunks.erase(key);
        dirtyQueued.erase(key);
    }

    void ClearRendered() {
        renderedChunks.clear();
    }

    void Rendered(int32_t cx, int32_t cy, int32_t cz) {
        renderedChunks.insert(PackChunkKey(cx, cy, cz));
    }

    bool IsRendered(int32_t cx, int32_t cy, int32_t cz) const {
        return renderedChunks.find(PackChunkKey(cx, cy, cz)) != renderedChunks.end();
    }

    bool IsChunkDirty(int32_t cx, int32_t cy, int32_t cz) const {
        auto ch = GetChunkAt(cx, cy, cz);
        return ch ? ch->IsChunkDirty() : false;
    }

    bool IsChunkLoaded(int32_t cx, int32_t cy, int32_t cz) const {
        auto ch = GetChunkAt(cx, cy, cz);
        return ch ? ch->IsChunkLoaded() : false;
    }

    void MarkChunkAsDirty(int32_t cx, int32_t cy, int32_t cz) {
        auto ch = GetChunkAt(cx, cy, cz);
        if (!ch) return;
        ch->MarkAsDirty();

        int64_t key = PackChunkKey(cx, cy, cz);
        if (dirtyQueued.insert(key).second) {
            dirtyQueue.push(key);
        }
    }

    void MarkAllChunksDirty() {
        for (auto &kv : chunks) {
            auto &chunk = kv.second;
            if (!chunk) continue;
            if (!chunk->IsChunkDirty()) {
                chunk->MarkAsDirty();
            }
            auto [cx, cy, cz] = UnpackChunkKey(kv.first);
            if (dirtyQueued.insert(kv.first).second) {
                dirtyQueue.push(kv.first);
            }
        }
    }
    
    void ProcessDirtyQueue(int32_t dirtyBudget, const std::function<void(int32_t,int32_t,int32_t)>& fn) {
        int32_t budget = dirtyBudget;

        while (budget > 0 && !dirtyQueue.empty()) {
            int64_t key = dirtyQueue.front();
            dirtyQueue.pop();
            dirtyQueued.erase(key);

            auto [cx, cy, cz] = UnpackChunkKey(key);
            fn(cx, cy, cz);
            --budget;
        }
    }

    void MarkNeighborChunksDirty(int32_t cx, int32_t cy, int32_t cz) {
        const int dirs[6][3] = {
            {-1, 0, 0}, {1, 0, 0},
            {0, -1, 0}, {0, 1, 0},
            {0, 0, -1}, {0, 0, 1}
        };

        for (auto& d : dirs) {
            MarkChunkAsDirty(cx + d[0], cy + d[1], cz + d[2]);
        }
    }

    void MarkChunkAsLoaded(int32_t cx, int32_t cy, int32_t cz) {
        auto ch = GetChunkAt(cx, cy, cz);
        if (!ch) return;
        ch->MarkAsLoaded();
    }

    void UnmarkChunkAsLoaded(int32_t cx, int32_t cy, int32_t cz) {
        auto ch = GetChunkAt(cx, cy, cz);
        if (!ch) return;
        ch->UnmarkAsLoaded();
    }

    void UnmarkChunkAsDirty(int32_t cx, int32_t cy, int32_t cz) {
        auto ch = GetChunkAt(cx, cy, cz);
        if (!ch) return;
        ch->UnmarkAsDirty();
    }

    void UnloadChunk(int32_t cx, int32_t cy, int32_t cz) {
        auto ch = GetChunkAt(cx, cy, cz);
        if (!ch) return;
        ch->UnloadChunk();
    }

    void UpdateChunkModel(int32_t cx, int32_t cy, int32_t cz, Model m) {
        auto ch = GetChunkAt(cx, cy, cz);
        if (!ch) return;
        ch->UpdateChunkModel(m);
    }

    void SetChunkMaterialTexture(int32_t cx, int32_t cy, int32_t cz, Texture2D& atlas) {
        auto ch = GetChunkAt(cx, cy, cz);
        if (!ch) return;
        ch->SetChunkMaterialTexture(atlas);
    }

    bool IsBlockTransparent(int worldX, int worldY, int worldZ) const {
        return GetBlockId(worldX, worldY, worldZ) == 0;
    }

    size_t GetChunkCount() const { return chunks.size(); }

    int GetBlockId(int64_t worldX, int64_t worldY, int64_t worldZ) const {
        auto [cx, lx] = WorldToChunkAndLocal((int)worldX);
        auto [cy, ly] = WorldToChunkAndLocal((int)worldY);
        auto [cz, lz] = WorldToChunkAndLocal((int)worldZ);

        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return 0;
        return chunk->GetBlock(lx, ly, lz);
    }

    bool SetBlock(int64_t worldX, int64_t worldY, int64_t worldZ, uint16_t id, SetblockActions action = SetblockActions::SET) {
        auto [cx, lx] = WorldToChunkAndLocal((int)worldX);
        auto [cy, ly] = WorldToChunkAndLocal((int)worldY);
        auto [cz, lz] = WorldToChunkAndLocal((int)worldZ);

        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return false;

        if (action == SetblockActions::SET) {
            chunk->SetBlockId(lx, ly, lz, id);
        } else if (action == SetblockActions::REPLACE) {
            if (chunk->GetBlock(lx, ly, lz) != 0) {
                chunk->SetBlockId(lx, ly, lz, id);
            }
        } else if (action == SetblockActions::KEEP) {
            if (chunk->GetBlock(lx, ly, lz) == 0) {
                chunk->SetBlockId(lx, ly, lz, id);
            }
        }

        if (chunkModifiedCb) {
            chunkModifiedCb(cx, cy, cz);
            if (lx == 0) chunkModifiedCb(cx - 1, cy, cz);
            if (lx == CHUNK_SIZE - 1) chunkModifiedCb(cx + 1, cy, cz);
            if (ly == 0) chunkModifiedCb(cx, cy - 1, cz);
            if (ly == CHUNK_SIZE - 1) chunkModifiedCb(cx, cy + 1, cz);
            if (lz == 0) chunkModifiedCb(cx, cy, cz - 1);
            if (lz == CHUNK_SIZE - 1) chunkModifiedCb(cx, cy, cz + 1);
        }

        return true;
    }

    static std::pair<int,int> WorldToChunkAndLocal(int w) {
        int c = (w >= 0) ? (w / CHUNK_SIZE) : ((w + 1) / CHUNK_SIZE - 1);
        int local = w - c * CHUNK_SIZE;
        return {c, local};
    }

    int64_t FillBlocks(int64_t ax, int64_t ay, int64_t az,
                    int64_t bx, int64_t by, int64_t bz,
                    const BlockFillActions& blockaction,
                    uint16_t id = 1,
                    int32_t blockLimit = 50000)
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

                    uint16_t current = GetBlockId(x, y, z);

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
    void ClearAllChunks() { 
        for (auto &kv : chunks) {
            if (kv.second) {
                kv.second->SetState(ChunkState::Unloading);
                kv.second->UnloadChunk();
            }
        }
        chunks.clear();
        renderedChunks.clear();
        while (!dirtyQueue.empty()) dirtyQueue.pop();
        dirtyQueued.clear();
    }

    std::unordered_map<int64_t, std::shared_ptr<Chunk>> chunks;
    std::unordered_set<int64_t> renderedChunks;
    std::queue<int64_t> dirtyQueue;
    std::unordered_set<int64_t> dirtyQueued;
    std::string name;
    Vector3 spawnpoint;
    ChunkModifiedCallback chunkModifiedCb;
};
