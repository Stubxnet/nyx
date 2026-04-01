#pragma once
#include <memory>
#include <array>
#include "Block.hpp"

constexpr int CHUNK_SIZE = 16;

class Chunk {
public:
    Chunk(int cx = 0, int cy = 0, int cz = 0) : cx(cx), cy(cy), cz(cz), empty(true) {
        InitializeBlocks();
    }

    std::shared_ptr<Block> GetBlock(int x, int y, int z) const {
        if (IsValidLocalPosition(x, y, z)) return blocks[x][y][z];
        return nullptr;
    }

    void SetBlockId(int x, int y, int z, int newId) {
        if (IsValidLocalPosition(x, y, z) && blocks[x][y][z]) {
            blocks[x][y][z]->SetId(newId);
            if (newId != 0) {
                empty = false;
            }
        }
    }

    int GetChunkX() const { return cx; }
    int GetChunkY() const { return cy; }
    int GetChunkZ() const { return cz; }
    bool IsChunkEmpty() const { return empty; }

private:
    // block storage in a 3D matrix
    std::array<std::array<std::array<std::shared_ptr<Block>, CHUNK_SIZE>, CHUNK_SIZE>, CHUNK_SIZE> blocks;
    // chunk position
    int cx, cy, cz;
    bool empty;

    void InitializeBlocks() {
        for (int x = 0; x < CHUNK_SIZE; ++x) {
            for (int y = 0; y < CHUNK_SIZE; ++y) {
                for (int z = 0; z < CHUNK_SIZE; ++z) {
                    Vector3 position = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
                    blocks[x][y][z] = std::make_shared<Block>(position, 0);
                }
            }
        }
    }

    bool IsValidLocalPosition(int x, int y, int z) const {
        return (x >= 0 && x < CHUNK_SIZE && y >= 0 && y < CHUNK_SIZE && z >= 0 && z < CHUNK_SIZE);
    }
};
