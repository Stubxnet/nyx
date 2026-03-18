#include <vector>
#include <string>
#include <memory>
#include "Chunk.hpp"

class World {
public:
    World(const std::string& name, Vector3 spawnpoint) 
        : name(name), spawnpoint(spawnpoint) {}

    int GetBlockId(int x, int y, int z) const {
        for (const auto& chunk : chunks) {
            if (chunk) {
                auto block = chunk->GetBlock(x, y, z);
                if (block) {
                    return block->GetId();
                }
            }
        }
        return -1;
    }

    void SetBlockId(int x, int y, int z, int id) {
        for (const auto& chunk : chunks) {
            if (chunk) {
                chunk->SetBlockId(x, y, z, id);
                return;
            }
        }
    }

    size_t GetChunkCount() const {
        return chunks.size();
    }

    std::shared_ptr<Chunk> GetChunk(size_t index) const {
        if (index < chunks.size()) {
            return chunks[index];
        }
        return nullptr;
    }

    bool IsBlockTransparent(int x, int y, int z) const {
        return GetBlockId(x, y, z) == 0;
    }

    void AddChunk(std::shared_ptr<Chunk> chunk) {
        chunks.push_back(chunk);
    }

private:
    std::vector<std::shared_ptr<Chunk>> chunks;
    std::string name;
    Vector3 spawnpoint;
};
