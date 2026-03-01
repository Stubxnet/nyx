#include <string>
#include <vector>
#include "Block.hpp"
#include "raylib.h"

class Chunk {
private:
    Vector3 position;
    std::string biome;
    std::string subBiome;
    std::vector<Block> blocks;

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
        return nullptr;
    }

    void setBlock(int x, int y, int z, int id) {
        for (auto& block : blocks) {
            Vector3 blockPos = block.getPosition();
            if (blockPos.x == x && blockPos.y == y && blockPos.z == z) {
                block.setId(id);
                return;
            }
        }
        return nullptr;
    }

    void addBlock(const Block& block) {
        blocks.push_back(block);
    }
};
