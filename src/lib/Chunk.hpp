#pragma once
#include <memory>
#include <array>
#include "Block.hpp"

class Chunk {
public:
    Chunk() {
        InitializeBlocks();
    }

    std::shared_ptr<Block> GetBlock(int x, int y, int z) const {
        if (IsValidPosition(x, y, z)) {
            return blocks[x][y][z];
        }
        return nullptr;
    }

    void SetBlockId(int x, int y, int z, int newId) {
        if (IsValidPosition(x, y, z) && blocks[x][y][z] != nullptr) {
            blocks[x][y][z]->SetId(newId);
        }
    }

private:
    std::array<std::array<std::array<std::shared_ptr<Block>, 16>, 16>, 16> blocks;

    void InitializeBlocks() {
        for (int x = 0; x < 16; ++x) {
            for (int y = 0; y < 16; ++y) {
                for (int z = 0; z < 16; ++z) {
                    Vector3 position = {static_cast<float>(x), static_cast<float>(y), static_cast<float>(z)};
                    blocks[x][y][z] = std::make_shared<Block>(position, 0);
                }
            }
        }
    }

    bool IsValidPosition(int x, int y, int z) const {
        return (x >= 0 && x < 16 && y >= 0 && y < 16 && z >= 0 && z < 16);
    }
};
