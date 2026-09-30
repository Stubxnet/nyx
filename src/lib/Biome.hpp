#pragma once

#include <cstdint>

using BlockId = uint16_t;

struct Biome
{
    BlockId top;
    BlockId soil;
    BlockId underground;

    float temperature;
    float moisture;
};

namespace Biomes
{
    inline constexpr Biome Ocean {
        2,      // top
        2,      // soil
        4,      // underground
        0.5f,   // temperature
        0.5f    // moisture
    };

    inline constexpr Biome Beach {
        4,
        4,
        4,
        0.7f,
        0.45f
    };

    inline constexpr Biome Plains {
        1,
        2,
        4,
        0.65f,
        0.5f
    };

    inline constexpr Biome Forest {
        1,
        2,
        4,
        0.55f,
        0.75f
    };

    inline constexpr Biome Desert {
        5,
        5,
        4,
        0.9f,
        0.15f
    };

    inline constexpr Biome SnowPlains {
        1,
        2,
        4,
        0.1f,
        0.5f
    };

    inline constexpr Biome Taiga {
        1,
        2,
        4,
        0.25f,
        0.7f
    };

    inline constexpr Biome Mountains {
        4,
        4,
        4,
        0.45f,
        0.45f
    };

    inline constexpr Biome Badlands {
        4,
        4,
        4,
        0.75f,
        0.25f
    };
}
