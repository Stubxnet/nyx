#pragma once

#include <cstdint>

using BlockId = uint16_t;

struct Biome {
    std::string id;

    // terrain
    BlockId top;
    BlockId soil;
    BlockId underground;
    int topBlocks = 0;
    int soilBlocks = 0;

    // selection parameters
    float temperature = 0.5f;
    float moisture = 0.5f;
    float temperatureWeight = 1.0f;
    float moistureWeight = 1.0f;

    BiomeSelectionType selectionType = BiomeSelectionType::Climate;
    int selectionPriority = 0;

    float maxContinentalness = 1.0f;
    float seaLevelBelow = 4.0f;
    float seaLevelAbove = 6.0f;

    std::vector<std::string> tags;
};
