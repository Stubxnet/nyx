#pragma once

#include "../core/constants.hpp"
#include "Biome.hpp"

#include <array>
#include <cstdint>

struct HeightmapCell
{
    int32_t surfaceY = 0;

    float temperature = 0.5f;
    float moisture = 0.5f;
    float erosion = 0.5f;
    float continentalness = 0.5f;

    Biome biome = Biomes::Plains;
};

class VerticalChunk
{
public:
    VerticalChunk(
        int32_t chunkX = 0,
        int32_t chunkZ = 0
    )   : cx(chunkX),
          cz(chunkZ) {}

    int32_t GetChunkX() const { return cx; }
    int32_t GetChunkZ() const { return cz; }

    HeightmapCell& GetCell(int x, int z) { return heightmap[x][z]; }

    const HeightmapCell& GetCell(int x, int z) const { return heightmap[x][z]; }

private:
    int32_t cx;
    int32_t cz;

    std::array<std::array<HeightmapCell, CHUNK_SIZE>,CHUNK_SIZE> heightmap;
};
