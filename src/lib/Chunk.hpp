#ifndef CHUNK_HPP
#define CHUNK_HPP

#include <string>
#include <vector>
#include "Block.hpp"
#include "raylib.h"

class Chunk {
private:
    Vector3 position;
    std::string biome;
    std::string subBiome;
    std::vector<Block> blocks; // TODO: replace this by a static type, fixed to 4096 blocks (16x16x16)

public:
    Chunk(float x, float y, float z, const std::string& biome, const std::string& subBiome) 
        : position({x, y, z}), biome(biome), subBiome(subBiome) {}

    ~Chunk() {}

    Vector3 getPosition() const { return position; }
    std::string getBiome() const { return biome; }
    std::string getSubBiome() const { return subBiome; }

    Block getBlock(int x, int y, int z) {
        for (const auto& block : blocks) {
            Vector3 blockPos = block.getPosition();
            if (blockPos.x == x && blockPos.y == y && blockPos.z == z) {
                return block;
            }
        }
    }

    bool setBlock(int x, int y, int z, int id) {
        for (auto& block : blocks) {
            Vector3 blockPos = block.getPosition();
            if (blockPos.x == x && blockPos.y == y && blockPos.z == z) {
                block.setId(id);
                return true;
            }
        }
        return false;
    }

    bool addBlock(const Block& block) {
        std::size_t length = blocks.size();
        if (length >= 4096) { // 4096 because chunks have a size of 16x16x16 blocks
            return false;
        } else {
            blocks.push_back(block);
            return true;
        }
    }
};

#endif // CHUNK_HPP