#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include <cstdint>
#include <vector>
#include <algorithm>
#include "Chunk.hpp"

static inline int64_t ChunkKey(int cx, int cy, int cz) {
    const int64_t MASK = (1LL << 21) - 1;
    auto enc = ( (int64_t)(cx & MASK) << 42 ) | ( (int64_t)(cy & MASK) << 21 ) | (int64_t)(cz & MASK);
    return enc;
}

enum class SetblockActions {
    SET,
    REPLACE,
    KEEP,
    BREAK
};

enum class BlockFillActions {
    SET,
    REPLACE,
    KEEP,
    BREAK,
    OUTLINE
};

class World {
public:
    World(const std::string& name, Vector3 spawnpoint) : name(name), spawnpoint(spawnpoint) {}

    void Rendered(int32_t cx, int32_t cy, int32_t cz) {
        renderedChunks.push_back(ChunkKey(cx, cy, cz));
    }

    bool IsRendered(int32_t cx, int32_t cy, int32_t cz) {
        int64_t key = ChunkKey(cx, cy, cz);
        return std::find(renderedChunks.begin(), renderedChunks.end(), key) != renderedChunks.end();
    }

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

    bool SetBlock(int worldX, int worldY, int worldZ, int id, SetblockActions action = SetblockActions::SET) {
        auto [cx, lx] = WorldToChunkAndLocal(worldX);
        auto [cy, ly] = WorldToChunkAndLocal(worldY);
        auto [cz, lz] = WorldToChunkAndLocal(worldZ);

        auto chunk = GetChunkAt(cx, cy, cz);
        if (!chunk) return false;

        int current = 0;
        auto block = chunk->GetBlock(lx, ly, lz);
        if (block) current = block->GetId();

        switch (action) {
            case SetblockActions::SET:
                chunk->SetBlockId(lx, ly, lz, id);
                return true;
            case SetblockActions::REPLACE:
                if (current != 0) {
                    chunk->SetBlockId(lx, ly, lz, id);
                    return true;
                }
                return false;
            case SetblockActions::KEEP:
                if (current == 0) {
                    chunk->SetBlockId(lx, ly, lz, id);
                    return true;
                }
                return false;
            case SetblockActions::BREAK:
                if (current != 0) {
                    chunk->SetBlockId(lx, ly, lz, 0);
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

    bool FillBlocks(int64_t ax, int64_t ay, int64_t az,
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
        if (volume <= 0) return false;
        if (volume > blockLimit) return false;

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

                    int current = GetBlockId((int) x, (int) y, (int) z);

                    switch (blockaction) {
                        case BlockFillActions::SET:
                            if (SetBlock((int)x, (int)y, (int)z, id, SetblockActions::SET))
                                ++placed;
                            break;
                        case BlockFillActions::REPLACE:
                            if (current != 0) {
                                if (SetBlock((int)x, (int)y, (int)z, id, SetblockActions::SET))
                                    ++placed;
                            }
                            break;
                        case BlockFillActions::KEEP:
                            if (current == 0) {
                                if (SetBlock((int)x, (int)y, (int)z, id, SetblockActions::SET))
                                    ++placed;
                            }
                            break;
                        case BlockFillActions::BREAK:
                            if (current != 0) {
                                if (SetBlock((int)x, (int)y, (int)z, 0, SetblockActions::SET))
                                    ++placed;
                            }
                            break;
                        case BlockFillActions::OUTLINE:
                            if (SetBlock((int)x, (int)y, (int)z, id, SetblockActions::SET))
                                ++placed;
                            break;
                    }

                    if (placed >= blockLimit) return false;
                }
            }
        }

        return true;
    }

private:
    std::unordered_map<int64_t, std::shared_ptr<Chunk>> chunks;
    std::vector<std::shared_ptr<Chunk>> chunkList; // optional list
    std::string name;
    std::vector<int64_t> renderedChunks;
    std::vector<int64_t> loadedChunks;
    Vector3 spawnpoint;
};
