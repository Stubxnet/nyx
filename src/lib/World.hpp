#include <string>
#include <vector>

#include "raylib.h"

#include "GameRules.hpp"
#include "Dimension.hpp"

class World {
private:
    std::string name;
    std::vector<Dimension> dimensions;
    int defaultSeed;
    std::string defaultPresets;
    bool hosted;
    GameRules defaultGamerules;

public:
    World(const std::string& name, int defaultSeed, const std::string& defaultPresets, bool hosted, const GameRules& defaultGamerules)
        : name(name), defaultSeed(defaultSeed), hosted(hosted), defaultGamerules(defaultGamerules) {}

    ~World() {}

    std::string getName() const { return name; }
    int getDefaultSeed() const { return defaultSeed; }
    std::string getDefaultPresets() const { return defaultPresets; }
    bool isHosted() const { return hosted; }
    GameRules getDefaultGamerules() const { return defaultGamerules; }

    void setName(const std::string& newName) { name = newName; }
    void setDefaultSeed(int newdefaultSeed) { defaultSeed = newdefaultSeed; }
    void setDefaultPresets(const std::string& newdefaultPresets) { defaultPresets = newdefaultPresets; }
    void setHosted(bool newHosted) { hosted = newHosted; }
    void setDefaultGamerules(const GameRules& newDefaultGamerules) { defaultGamerules = newDefaultGamerules; }

    Dimension* getDimension(const std::string& dimensionName) {
        for (auto& dimension : dimensions) {
            std::string name = dimension.getName();
            if (name == dimensionName) {
                return &dimension;
            }
        }
        return nullptr;
    }

    int getDimensionsCount() const {
        return static_cast<int>(dimensions.size());
    }

    void addDimension(const Dimension& dimension) {
        dimensions.push_back(dimension);
    }
};
