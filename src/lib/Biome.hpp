#pragma once

#include <cstdint>

using BlockId = uint16_t;

enum class BiomeId : uint8_t
{
    Ocean,
    Beach,
    Plains,
    Forest,
    Desert,
    SnowPlains,
    Taiga,
    Mountains,
    Badlands
};

struct Biome
{
    BiomeId id;

    BlockId top;
    BlockId soil;
    BlockId underground;

    uint8_t topBlocks;
    uint8_t soilBlocks;
};

namespace Biomes
{
    inline constexpr Biome Ocean {
        BiomeId::Ocean,
        2, 2, 4,
        1, 1
    };

    inline constexpr Biome Beach {
        BiomeId::Beach,
        5, 5, 4,
        1, 20
    };

    inline constexpr Biome Plains {
        BiomeId::Plains,
        1, 2, 4,
        1, 2
    };

    inline constexpr Biome Forest {
        BiomeId::Forest,
        1, 2, 4,
        1, 3
    };

    inline constexpr Biome Desert {
        BiomeId::Desert,
        5, 5, 4,
        1, 20
    };

    inline constexpr Biome SnowPlains {
        BiomeId::SnowPlains,
        1, 2, 4,
        1, 1
    };

    inline constexpr Biome Taiga {
        BiomeId::Taiga,
        1, 2, 4,
        1, 1
    };

    inline constexpr Biome Mountains {
        BiomeId::Mountains,
        4, 4, 4,
        1, 1
    };

    inline constexpr Biome Badlands {
        BiomeId::Badlands,
        5, 4, 4,
        1, 1
    };
}

static const char* GetBiomeName(BiomeId id) {
    switch (id) {
        case BiomeId::Ocean:      return "Ocean";
        case BiomeId::Beach:      return "Beach";
        case BiomeId::Plains:     return "Plains";
        case BiomeId::Forest:     return "Forest";
        case BiomeId::Desert:     return "Desert";
        case BiomeId::SnowPlains: return "Snow Plains";
        case BiomeId::Taiga:      return "Taiga";
        case BiomeId::Mountains:  return "Mountains";
        case BiomeId::Badlands:   return "Badlands";
    }

    return "Unknown";
}
