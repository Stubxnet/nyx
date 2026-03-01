#include <string>
#include <vector>
#include "Chunk.hpp"
//#include "Gamerules.hpp"

class World {
private:
    std::string name;
    int seed;
    std::string presets;
    bool hosted;
    std::vector<Chunk> chunks;
//    Gamerules gamerules;

public:
    World(const std::string& name, int seed, const std::string& presets, bool hosted, const Gamerules& gamerules)
        : name(name), seed(seed), hosted(hosted), gamerules(gamerules) {}

    ~World() {}

    std::string getName() const { return name; }
    int getSeed() const { return seed; }
    std::string getPresets() const { return presets }
    bool isHosted() const { return hosted; }
    std::vector<Chunk> getChunks() const { return chunks; }
//    Gamerules getGamerules() const { return gamerules; }

    void setName(const std::string& newName) { name = newName; }
    void setSeed(int newSeed) { seed = newSeed; }
    void setPresets(const std::string& newPresets) { presets = newPresets }
    void setHosted(bool newHosted) { hosted = newHosted; }
    void setChunks(const std::vector<Chunk>& newChunks) { chunks = newChunks; }
//    void setGamerules(const Gamerules& newGamerules) { gamerules = newGamerules; }

    Chunk* getChunk(int x, int y, int z) {
        for (auto& chunk : chunks) {
            Vector3 position = chunk.getPosition();
            if (position.getX() == x && position.getY() == y && position.getZ() == z) {
                return &chunk;
            }
        }
        return nullptr;
    }
};
