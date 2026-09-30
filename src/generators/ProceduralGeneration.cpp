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

std::shared_ptr<VerticalChunk> GenerateChunkHeightmap(
    int32_t cx,
    int32_t cz,
    int32_t seed)
{
    auto result = std::make_shared<VerticalChunk>(cx, cz);

    FastNoiseLite continentNoise, temperatureNoise, moistureNoise;
    FastNoiseLite erosionNoise, warpNoise, detailNoise;

    ConfigureNoise(continentNoise, seed,     0.00020f, 3);
    ConfigureNoise(temperatureNoise, seed+1, 1.0f / BIOME_SIZE, 3);
    ConfigureNoise(moistureNoise,    seed+2, 1.0f / BIOME_SIZE, 3);
    ConfigureNoise(erosionNoise,     seed+3, 1.0f / (BIOME_SIZE * 0.65f), 2);
    ConfigureNoise(warpNoise,        seed+4, 0.00025f, 2);
    ConfigureNoise(detailNoise,       seed+5, 0.003f, 2);

    for (int localX = 0; localX < CHUNK_SIZE; ++localX)
    {
        for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ)
        {
            const int worldX = GetWorldFromChunkAndLocal(cx, localX);
            const int worldZ = GetWorldFromChunkAndLocal(cz, localZ);

            const float x = worldX * WORLD_SCALE;
            const float z = worldZ * WORLD_SCALE;

            const float warpedX = x + warpNoise.GetNoise(x, z) * 48.0f;
            const float warpedZ =
                z + warpNoise.GetNoise(x + 1000.0f, z + 1000.0f) * 48.0f;

            const float continental = Clamp01(Remap01(
                continentNoise.GetNoise(warpedX, warpedZ)));

            const float temperature = Clamp01(Remap01(
                temperatureNoise.GetNoise(x, z)));

            const float moisture = Clamp01(Remap01(
                moistureNoise.GetNoise(x, z)));

            const float erosion = Clamp01(Remap01(
                erosionNoise.GetNoise(warpedX, warpedZ)));

            const float detail = detailNoise.GetNoise(
                warpedX * 2.2f, warpedZ * 2.2f);

            const float landMask = continental - SEA_CONTINENTALNESS;
            const bool ocean = landMask < 0.0f;

            float height01;

            if (ocean)
            {
                const float depth = Clamp01(-landMask * 2.0f);
                height01 = 0.30f - depth * 0.10f + detail * 0.015f;
            }
            else
            {
                const float landAmount = Clamp01(
                    landMask / (1.0f - SEA_CONTINENTALNESS));

                const float mountainInput =
                    (continental - 0.62f) * 2.0f +
                    (erosion - 0.5f) * 0.55f;

                const float mountainFactor =
                    SmoothStep(0.0f, 1.0f, mountainInput);

                const float mountainRelief =
                    mountainFactor * mountainFactor * 0.40f;

                height01 =
                    0.34f +
                    landAmount * 0.20f +
                    (erosion - 0.5f) * 0.08f +
                    mountainRelief +
                    detail * 0.025f;
            }

            height01 = Clamp01(height01);

            const int32_t surfaceY = ocean
                ? SEA_LEVEL - static_cast<int32_t>(
                    Clamp01((0.30f - height01) / 0.10f) *
                    MAX_OCEAN_DEPTH)
                : MIN_SURFACE_Y + static_cast<int32_t>(
                    height01 * (MAX_SURFACE_Y - MIN_SURFACE_Y));

            Biome biome = SelectBiome(
                height01,
                temperature,
                moisture,
                erosion,
                ocean);

            if (!ocean && surfaceY <= SEA_LEVEL + 2)
                biome = Biomes::Beach;

            auto& cell = result->GetCell(localX, localZ);
            cell.surfaceY = surfaceY;
            cell.temperature = temperature;
            cell.moisture = moisture;
            cell.erosion = erosion;
            cell.continentalness = continental;
            cell.biome = biome;
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
