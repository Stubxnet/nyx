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

    float temperature;
    float moisture;

    float terrainBase;
    float terrainScale;
    float terrainRoughness;
};

namespace Biomes
{
    inline constexpr Biome Ocean {
        BiomeId::Ocean,
        2, 2, 4,
        0.5f, 0.5f,
        0.22f, 0.035f, 0.10f
    };

    inline constexpr Biome Beach {
        BiomeId::Beach,
        4, 4, 4,
        0.7f, 0.45f,
        0.395f, 0.018f, 0.05f
    };

    inline constexpr Biome Plains {
        BiomeId::Plains,
        1, 2, 4,
        0.65f, 0.5f,
        0.435f, 0.035f, 0.12f
    };

    inline constexpr Biome Forest {
        BiomeId::Forest,
        1, 2, 4,
        0.55f, 0.75f,
        0.455f, 0.075f, 0.28f
    };

    inline constexpr Biome Desert {
        BiomeId::Desert,
        5, 5, 4,
        0.9f, 0.15f,
        0.425f, 0.040f, 0.12f
    };

    inline constexpr Biome SnowPlains {
        BiomeId::SnowPlains,
        1, 2, 4,
        0.1f, 0.5f,
        0.445f, 0.040f, 0.14f
    };

    inline constexpr Biome Taiga {
        BiomeId::Taiga,
        1, 2, 4,
        0.25f, 0.7f,
        0.465f, 0.080f, 0.30f
    };

    inline constexpr Biome Mountains {
        BiomeId::Mountains,
        4, 4, 4,
        0.45f, 0.45f,
        0.52f, 0.30f, 0.90f
    };

    inline constexpr Biome Badlands {
        BiomeId::Badlands,
        4, 4, 4,
        0.75f, 0.25f,
        0.48f, 0.16f, 0.65f
    };
}
