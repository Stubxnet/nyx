#ifndef DIMENSION_HPP
#define DIMENSION_HPP

#include <string>
#include <vector>

#include "raylib.h"

#include "Chunk.hpp"
#include "GameRules.hpp"

class Dimension {
private:
    std::string name;
    int seed;
    std::string presets;
    std::vector<Chunk> chunks;
    GameRules gamerules;

public:
    Dimension(const std::string& name, int seed = 0, const std::string& presets = "", bool hosted = false, const GameRules& gamerules = GameRules())
    : name(name), seed(seed), presets(presets), gamerules(gamerules) {}

    std::string getName() const { return name; }
    int getSeed() const { return seed; }
    std::string getPresets() const { return presets; }
    GameRules getGameRules() const { return gamerules; }

    void setName(const std::string& newName) { name = newName; }
    void setSeed(int newSeed) { seed = newSeed; }
    void setPresets(const std::string& newPresets) { presets = newPresets; }
    void setGameRules(const GameRules& newGameRules) { gamerules = newGameRules; }

    void addChunk(const Chunk& chunk) {
        chunks.push_back(chunk);
    }

    Chunk* getChunk(int x, int y, int z) {
        for (auto& chunk : chunks) {
            Vector3 position = chunk.getPosition();
            if (position.x == x && position.y == y && position.z == z) {
                return &chunk;
            }
        }
        return nullptr;
    }

    int getChunksCount() const {
        return static_cast<int>(chunks.size());
    }
};

#endif // DIMENSION_HPP