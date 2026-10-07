#include "../core/constants.hpp"
#include "../lib/World.hpp"

#include "../../include/FastNoiseLite.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace TerrainGeneration {

struct NoiseContext {
    uint32_t seed = 0;
    bool initialized = false;

    FastNoiseLite continental;
    FastNoiseLite temperature;
    FastNoiseLite moisture;
    FastNoiseLite erosion;
    FastNoiseLite weirdness;
    FastNoiseLite detail;
    FastNoiseLite oceanDepth;
    FastNoiseLite oceanDetail;
    FastNoiseLite warpX;
    FastNoiseLite warpZ;
};

struct TerrainSample {
    float continentalness = 0.0f;
    float temperature = 0.0f;
    float moisture = 0.0f;
    float erosion = 0.0f;
    float weirdness = 0.0f;

    float landMask = 0.0f;
    float mountainMask = 0.0f;
    float ridgeMask = 0.0f;
    float valleyMask = 0.0f;

    float height01 = 0.0f;

    bool ocean = false;
    const Biome* biome = nullptr;
};

} // namespace TerrainGeneration

namespace TerrainGeneration {

namespace {

NoiseContext g_noise;

float Clamp01(float value) { return std::clamp(value, 0.0f, 1.0f); }

float Remap01(float value) { return value * 0.5f + 0.5f; }

float SmoothStep(float edge0, float edge1, float value) {
    if (edge0 == edge1) { return value < edge0 ? 0.0f : 1.0f; }

    const float t = Clamp01((value - edge0) / (edge1 - edge0));

    return t * t * (3.0f - 2.0f * t);
}

float InverseSmoothStep(float edge0, float edge1, float value) { return 1.0f - SmoothStep(edge0, edge1, value); }

float RemapLinearClamped(float value, float fromMin, float fromMax, float toMin, float toMax) {
    if (fromMin == fromMax) { return toMin; }

    const float t = Clamp01((value - fromMin) / (fromMax - fromMin));

    return toMin + (toMax - toMin) * t;
}

float GetSeaHeight01() { return RemapLinearClamped(static_cast<float>(SEA_LEVEL), static_cast<float>(MIN_SURFACE_Y), static_cast<float>(MAX_SURFACE_Y), 0.0f, 1.0f); }

float GetTerrainRange() { return static_cast<float>(MAX_SURFACE_Y - MIN_SURFACE_Y); }

uint32_t HashSeed(uint32_t seed, uint32_t stream) {
    uint32_t value = seed ^ (stream + 0x9E3779B9u);

    value ^= value >> 16;
    value *= 0x85EBCA6Bu;
    value ^= value >> 13;
    value *= 0xC2B2AE35u;
    value ^= value >> 16;

    return value;
}

void ConfigureNoise(FastNoiseLite& noise, uint32_t seed, float frequency, int octaves) {
    noise.SetSeed(static_cast<int32_t>(seed));
    noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    noise.SetFractalType(FastNoiseLite::FractalType_FBm);
    noise.SetFractalOctaves(octaves);
    noise.SetFractalLacunarity(2.0f);
    noise.SetFractalGain(0.5f);
    noise.SetFrequency(frequency);
}

float Sample01(const FastNoiseLite& noise, float x, float z) { return Clamp01(Remap01(noise.GetNoise(x, z))); }

const Biome* SelectLandBiome(const TerrainSample& sample) {
    const float seaHeight01 = GetSeaHeight01();

    const float terrainRange = GetTerrainRange();

    constexpr float beachBelowBlocks = 4.0f;
    constexpr float beachAboveBlocks = 6.0f;

    const float beachBelow = beachBelowBlocks / terrainRange;

    const float beachAbove = beachAboveBlocks / terrainRange;

    const bool nearSea = sample.height01 >= seaHeight01 - beachBelow && sample.height01 <= seaHeight01 + beachAbove;

    if (nearSea && sample.continentalness < COAST_END) { return &Biomes::Beach; }

    if (sample.mountainMask > 0.55f) {
        if (sample.temperature < 0.28f || sample.height01 > 0.72f) { return &Biomes::SnowPlains; }

        return &Biomes::Mountains;
    }

    if (sample.temperature < 0.24f) { return &Biomes::SnowPlains; }

    if (sample.temperature < 0.42f) {
        if (sample.moisture > 0.48f) { return &Biomes::Taiga; }

        return &Biomes::Plains;
    }

    if (sample.temperature > 0.68f && sample.moisture < 0.30f) {
        if (sample.ridgeMask > 0.60f && sample.erosion > 0.48f) { return &Biomes::Badlands; }

        return &Biomes::Desert;
    }

    if (sample.moisture > 0.64f) { return &Biomes::Forest; }

    return &Biomes::Plains;
}

float CalculateLandHeight(const TerrainSample& sample, float seaHeight01, float detail) {
    const float continentalLand = SmoothStep(SEA_CONTINENTALNESS, 0.88f, sample.continentalness);

    const float lowErosion = InverseSmoothStep(0.20f, 0.75f, sample.erosion);

    const float erosionHeight = lowErosion * 0.025f;

    const float ridgeValue = std::abs(sample.weirdness * 2.0f - 1.0f);

    const float ridgeMask = SmoothStep(0.55f, 0.88f, ridgeValue);

    const float centralWeirdness = 1.0f - ridgeValue;

    const float valleyMask = SmoothStep(0.30f, 0.85f, centralWeirdness);

    const float mountainClimate = 0.45f * continentalLand + 0.30f * lowErosion + 0.25f * ridgeMask;

    const float mountainMask = continentalLand * SmoothStep(0.58f, 0.78f, mountainClimate);

    const float mountainHeight = mountainMask * 0.22f;

    const float valleyDepth = valleyMask * mountainMask * 0.050f;

    const float continentalHeight = RemapLinearClamped(sample.continentalness, SEA_CONTINENTALNESS, 0.88f, 0.0f, 0.115f);

    const float localDetail = detail * 0.018f;

    const float climateVariation = (sample.moisture - 0.5f) * 0.008f + (0.5f - sample.erosion) * 0.010f;

    const float result = seaHeight01 + continentalHeight * continentalLand + erosionHeight * continentalLand + mountainHeight - valleyDepth + localDetail + climateVariation;

    return Clamp01(result);
}

float CalculateOceanHeight(float seaHeight01, float x, float z) {
    const float basinNoise = Sample01(g_noise.oceanDepth, x, z);

    const float detailNoise = g_noise.oceanDetail.GetNoise(x, z);

    const float basinMask = SmoothStep(0.20f, 0.85f, basinNoise);

    const float terrainRange = GetTerrainRange();

    const float availableDepth = static_cast<float>(SEA_LEVEL - MIN_SURFACE_Y);

    const float maxDepth = std::max(1.0f, availableDepth * 0.90f);

    const float detailAmplitude = std::min(6.0f, maxDepth * 0.10f);

    const float depthBlocks = basinMask * maxDepth + detailNoise * detailAmplitude;

    const float result = seaHeight01 - depthBlocks / terrainRange;

    return std::clamp(result, 0.0f, seaHeight01);
}

} // namespace

void Configure(uint32_t seed) {
    g_noise.seed = seed;

    ConfigureNoise(g_noise.continental, HashSeed(seed, 0), CONTINENT_FREQUENCY, 5);

    ConfigureNoise(g_noise.temperature, HashSeed(seed, 1), CLIMATE_FREQUENCY, 3);

    ConfigureNoise(g_noise.moisture, HashSeed(seed, 2), CLIMATE_FREQUENCY, 3);

    ConfigureNoise(g_noise.erosion, HashSeed(seed, 3), EROSION_FREQUENCY, 4);

    ConfigureNoise(g_noise.weirdness, HashSeed(seed, 4), WEIRDNESS_FREQUENCY, 4);

    ConfigureNoise(g_noise.detail, HashSeed(seed, 5), DETAIL_FREQUENCY, 4);

    ConfigureNoise(g_noise.warpX, HashSeed(seed, 6), WARP_FREQUENCY, 2);

    ConfigureNoise(g_noise.warpZ, HashSeed(seed, 7), WARP_FREQUENCY, 2);

    ConfigureNoise(g_noise.oceanDepth, HashSeed(seed, 8), OCEAN_DEPTH_FREQUENCY, 4);

    ConfigureNoise(g_noise.oceanDetail, HashSeed(seed, 9), OCEAN_DETAIL_FREQUENCY, 4);

    g_noise.initialized = true;
}

void EnsureInitialized(uint32_t seed) {
    if (!g_noise.initialized || g_noise.seed != seed) { Configure(seed); }
}

TerrainSample Sample(int64_t worldX, int64_t worldZ) {
    TerrainSample sample;

    const double baseXDouble = static_cast<double>(worldX) * static_cast<double>(WORLD_SCALE);

    const double baseZDouble = static_cast<double>(worldZ) * static_cast<double>(WORLD_SCALE);

    const float baseX = static_cast<float>(baseXDouble);

    const float baseZ = static_cast<float>(baseZDouble);

    const float warpX = g_noise.warpX.GetNoise(baseX, baseZ) * WORLD_WARP_DISTANCE;

    const float warpZ = g_noise.warpZ.GetNoise(baseX + 1000.0f, baseZ + 1000.0f) * WORLD_WARP_DISTANCE;

    const float x = baseX + warpX;

    const float z = baseZ + warpZ;

    const float detail = g_noise.detail.GetNoise(x, z);

    sample.continentalness = Sample01(g_noise.continental, x, z);

    sample.temperature = Sample01(g_noise.temperature, x, z);

    sample.moisture = Sample01(g_noise.moisture, x, z);

    sample.erosion = Sample01(g_noise.erosion, x, z);

    sample.weirdness = Sample01(g_noise.weirdness, x, z);

    // Décision unique et cohérente.
    // La hauteur et le biome utilisent cette même classification.
    sample.ocean = sample.continentalness < SEA_CONTINENTALNESS;

    sample.landMask = SmoothStep(SEA_CONTINENTALNESS - 0.08f, SEA_CONTINENTALNESS + 0.10f, sample.continentalness);

    const float seaHeight01 = GetSeaHeight01();

    const float ridgeValue = std::abs(sample.weirdness * 2.0f - 1.0f);

    sample.ridgeMask = SmoothStep(0.55f, 0.88f, ridgeValue);

    sample.valleyMask = SmoothStep(0.30f, 0.85f, 1.0f - ridgeValue);

    const float lowErosion = InverseSmoothStep(0.20f, 0.75f, sample.erosion);

    const float continentalLand = SmoothStep(SEA_CONTINENTALNESS, 0.88f, sample.continentalness);

    const float mountainClimate = 0.45f * continentalLand + 0.30f * lowErosion + 0.25f * sample.ridgeMask;

    sample.mountainMask = continentalLand * SmoothStep(0.58f, 0.78f, mountainClimate);

    const float landHeight = CalculateLandHeight(sample, seaHeight01, detail);

    const float oceanHeight = CalculateOceanHeight(seaHeight01, x, z);

    if (sample.ocean) {
        sample.height01 = oceanHeight;
    } else {
        sample.height01 = landHeight;
    }

    sample.height01 = Clamp01(sample.height01);

    if (sample.ocean) {
        sample.biome = &Biomes::Ocean;
    } else {
        sample.biome = SelectLandBiome(sample);
    }

    return sample;
}

int32_t ComputeSurfaceY(const TerrainSample& sample, int64_t worldX, int64_t worldZ) {
    (void)worldX;
    (void)worldZ;

    const double terrainRange = static_cast<double>(MAX_SURFACE_Y - MIN_SURFACE_Y);

    const double surface = static_cast<double>(MIN_SURFACE_Y) + static_cast<double>(sample.height01) * terrainRange;

    const double clampedSurface = std::clamp(surface, static_cast<double>(MIN_SURFACE_Y), static_cast<double>(MAX_SURFACE_Y));

    return static_cast<int32_t>(std::lround(clampedSurface));
}

} // namespace TerrainGeneration
