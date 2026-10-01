#include "../core/constants.hpp"
#include "../lib/Chunk.hpp"
#include "../lib/VerticalChunk.hpp"
#include "../lib/World.hpp"

#include "../../include/FastNoiseLite.h"

#include <algorithm>
#include <cstdint>
#include <memory>

namespace
{
    int32_t GetWorldFromChunkAndLocal(int32_t chunk, int local) { return chunk * CHUNK_SIZE + local; }

    constexpr float SEA_CONTINENTALNESS = 0.52f;

    float Remap01(float v) { return v * 0.5f + 0.5f; }

    float Clamp01(float v) { return std::clamp(v, 0.0f, 1.0f); }

    float SmoothStep(float a, float b, float v)
    {
        if (a == b) return v < a ? 0.0f : 1.0f;

        const float t = Clamp01((v - a) / (b - a));
        return t * t * (3.0f - 2.0f * t);
    }

    void ConfigureNoise(
        FastNoiseLite& noise,
        int32_t seed,
        float frequency,
        int octaves)
    {
        noise.SetSeed(seed);
        noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
        noise.SetFractalType(FastNoiseLite::FractalType_FBm);
        noise.SetFrequency(frequency);
        noise.SetFractalOctaves(octaves);
    }

    Biome SelectBiome(
        float height,
        float temperature,
        float moisture,
        float erosion,
        bool ocean)
    {
        if (ocean) return Biomes::Ocean;
        if (height > 0.78f) return Biomes::Mountains;
        if (temperature < 0.22f) return Biomes::SnowPlains;
        if (temperature > 0.72f && moisture < 0.30f)
            return Biomes::Desert;
        if (erosion > 0.70f && temperature > 0.55f && moisture < 0.45f)
            return Biomes::Badlands;
        if (temperature < 0.38f)
            return moisture > 0.55f ? Biomes::Taiga : Biomes::Plains;
        return moisture > 0.66f ? Biomes::Forest : Biomes::Plains;
    }
}

std::shared_ptr<VerticalChunk> GenerateChunkHeightmap(int32_t cx, int32_t cz, int32_t seed) {
    auto result = std::make_shared<VerticalChunk>(cx, cz);

    FastNoiseLite continentNoise, temperatureNoise, moistureNoise, erosionNoise, warpNoise, detailNoise, biomeNoise;
    ConfigureNoise(continentNoise,  seed,     0.00020f / 2.0f, 3);
    ConfigureNoise(temperatureNoise, seed + 1, 1.0f / (BIOME_SIZE * 3.0f), 2);
    ConfigureNoise(moistureNoise,    seed + 2, 1.0f / (BIOME_SIZE * 3.0f), 2);
    ConfigureNoise(erosionNoise,     seed + 3, 1.0f / (BIOME_SIZE * 2.0f), 2);
    ConfigureNoise(warpNoise,        seed + 4, 0.00025f / 2.0f, 2);
    ConfigureNoise(detailNoise,      seed + 5, 0.0045f, 4);


    biomeNoise.SetSeed(seed + 6);
    biomeNoise.SetNoiseType(FastNoiseLite::NoiseType_Cellular);
    biomeNoise.SetCellularDistanceFunction(FastNoiseLite::CellularDistanceFunction_EuclideanSq);
    biomeNoise.SetCellularReturnType(FastNoiseLite::CellularReturnType_CellValue);
    biomeNoise.SetCellularJitter(0.85f);
    biomeNoise.SetFrequency(1.0f / (BIOME_SIZE * 4.0f));
    biomeNoise.SetFractalType(FastNoiseLite::FractalType_None);

    const float seaHeight01 = Clamp01(static_cast<float>(SEA_LEVEL - MIN_SURFACE_Y) / static_cast<float>(MAX_SURFACE_Y - MIN_SURFACE_Y));

    const auto GetBiomeProfile = [seaHeight01](const Biome* biome, float& base, float& amplitude, float& roughness) {
        if (biome == &Biomes::Plains) { base = 0.425f; amplitude = 0.018f; roughness = 0.025f; }
        else if (biome == &Biomes::Forest) { base = 0.445f; amplitude = 0.045f; roughness = 0.070f; }
        else if (biome == &Biomes::Desert) { base = 0.420f; amplitude = 0.025f; roughness = 0.040f; }
        else if (biome == &Biomes::SnowPlains) { base = 0.435f; amplitude = 0.025f; roughness = 0.045f; }
        else if (biome == &Biomes::Taiga) { base = 0.455f; amplitude = 0.050f; roughness = 0.080f; }
        else if (biome == &Biomes::Badlands) { base = 0.470f; amplitude = 0.105f; roughness = 0.150f; }
        else if (biome == &Biomes::Mountains) { base = 0.500f; amplitude = 0.220f; roughness = 0.300f; }
        else if (biome == &Biomes::Beach) { base = seaHeight01 + 0.008f; amplitude = 0.010f; roughness = 0.015f; }
        else { base = seaHeight01 - 0.08f; amplitude = 0.020f; roughness = 0.030f; }
    };

    for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
        for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ) {
            const int worldX = GetWorldFromChunkAndLocal(cx, localX);
            const int worldZ = GetWorldFromChunkAndLocal(cz, localZ);
            const float x = worldX * WORLD_SCALE, z = worldZ * WORLD_SCALE;
            const float warpedX = x + warpNoise.GetNoise(x, z) * 48.0f;
            const float warpedZ = z + warpNoise.GetNoise(x + 1000.0f, z + 1000.0f) * 48.0f;

            const float continental = Clamp01(Remap01(continentNoise.GetNoise(warpedX, warpedZ)));
            const float temperature = Clamp01(Remap01(temperatureNoise.GetNoise(warpedX, warpedZ)));
            const float moisture = Clamp01(Remap01(moistureNoise.GetNoise(warpedX, warpedZ)));
            const float erosion = Clamp01(Remap01(erosionNoise.GetNoise(warpedX, warpedZ)));
            const float detail = detailNoise.GetNoise(warpedX, warpedZ) * 0.5f;
            const bool ocean = continental < SEA_CONTINENTALNESS;
            const Biome* biome = ocean ? &Biomes::Ocean : &Biomes::Plains;

            if (!ocean) {
                const float biomeValue = Clamp01(Remap01(biomeNoise.GetNoise(warpedX, warpedZ)));
                const int biomeSlot = std::min(6, static_cast<int>(biomeValue * 7.0f));

                switch (biomeSlot) {
                    case 0: biome = &Biomes::Plains; break;
                    case 1: biome = &Biomes::Forest; break;
                    case 2: biome = &Biomes::Desert; break;
                    case 3: biome = &Biomes::SnowPlains; break;
                    case 4: biome = &Biomes::Taiga; break;
                    case 5: biome = &Biomes::Badlands; break;
                    default: biome = &Biomes::Mountains; break;
                }

                if ((biome == &Biomes::Desert && temperature < 0.35f) ||
                    (biome == &Biomes::SnowPlains && temperature > 0.70f) ||
                    (biome == &Biomes::Taiga && temperature > 0.72f) ||
                    (biome == &Biomes::Badlands && erosion < 0.30f)) {
                    biome = &Biomes::Plains;
                }
            }

            float height01 = seaHeight01;
            if (ocean) {
                const float depthFactor = Clamp01((SEA_CONTINENTALNESS - continental) / 0.20f);
                height01 = seaHeight01 - depthFactor * 0.12f + detail * 0.008f;
            } else {
                float base, amplitude, roughness;
                GetBiomeProfile(biome, base, amplitude, roughness);
                const float biomeHeight = base + detail * amplitude + (erosion - 0.5f) * roughness;
                const float coastFactor = SmoothStep(SEA_CONTINENTALNESS, SEA_CONTINENTALNESS + 0.16f, continental);
                height01 = seaHeight01 + (biomeHeight - seaHeight01) * coastFactor;
            }

            height01 = Clamp01(height01);
            int32_t surfaceY = ocean ?
                SEA_LEVEL - static_cast<int32_t>(Clamp01((seaHeight01 - height01) / 0.12f) * MAX_OCEAN_DEPTH) :
                MIN_SURFACE_Y + static_cast<int32_t>(height01 * (MAX_SURFACE_Y - MIN_SURFACE_Y));

            if (!ocean && surfaceY >= SEA_LEVEL - 1 && surfaceY <= SEA_LEVEL + 2) biome = &Biomes::Beach;

            auto& cell = result->GetCell(localX, localZ);
            cell = {surfaceY, temperature, moisture, erosion, continental, *biome};
        }
    }

    return result;
}

std::shared_ptr<Chunk> GenerateChunkBlocks(
    const VerticalChunk& verticalChunk,
    int32_t cy)
{
    const int32_t cx = verticalChunk.GetChunkX();
    const int32_t cz = verticalChunk.GetChunkZ();
    const int32_t baseY = cy * CHUNK_SIZE;
    const int32_t endY = baseY + CHUNK_SIZE;

    auto chunk = std::make_shared<Chunk>(cx, cy, cz);

    for (int x = 0; x < CHUNK_SIZE; ++x)
    {
        for (int z = 0; z < CHUNK_SIZE; ++z)
        {
            const auto& cell = verticalChunk.GetCell(x, z);
            const auto& biome = cell.biome;
            const int32_t surfaceY = cell.surfaceY;

            if (baseY > surfaceY)
                continue;

            if (endY <= surfaceY)
            {
                for (int y = 0; y < CHUNK_SIZE; ++y)
                    chunk->SetBlockId(x, y, z, biome.underground);

                continue;
            }

            for (int y = 0; y < CHUNK_SIZE; ++y)
            {
                const int32_t worldY = baseY + y;
                const BlockId block =
                    worldY == surfaceY     ? biome.top :
                    worldY == surfaceY - 1 ? biome.soil :
                    worldY < surfaceY      ? biome.underground :
                                              BlockId{0};

                if (block != 0)
                    chunk->SetBlockId(x, y, z, block);
            }
        }
    }

    chunk->UnmarkAsDirty();
    chunk->SetState(ChunkState::Generated);
    return chunk;
}
