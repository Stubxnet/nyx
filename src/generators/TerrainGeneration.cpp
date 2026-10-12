#include "../core/constants.hpp"
#include "../lib/World.hpp"

#include "../../include/FastNoiseLite.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <limits>

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

    float surfaceY = 0.0f;

    float height01 = 0.0f;

    bool ocean = false;
    const Biome* biome = nullptr;
};

} // namespace TerrainGeneration

namespace TerrainGeneration {

namespace {

NoiseContext g_noise;

// -----------------------------------------------------------------------------
// math helpers
// -----------------------------------------------------------------------------

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

float Lerp(float a, float b, float t) { return a + (b - a) * t; }

float SmoothLerp(float a, float b, float t) { return Lerp(a, b, t * t * (3.0f - 2.0f * t)); }

// -----------------------------------------------------------------------------
// generic curve
// -----------------------------------------------------------------------------

struct CurvePoint {
    float x;
    float y;
};

float EvaluateCurve(const std::array<CurvePoint, 9>& points, float value) {
    if (value <= points.front().x) { return points.front().y; }

    if (value >= points.back().x) { return points.back().y; }

    for (std::size_t i = 1; i < points.size(); ++i) {
        const CurvePoint& left = points[i - 1];
        const CurvePoint& right = points[i];

        if (value <= right.x) {
            const float t = Clamp01((value - left.x) / (right.x - left.x));

            return SmoothLerp(left.y, right.y, t);
        }
    }

    return points.back().y;
}

// -----------------------------------------------------------------------------
// seed and noise conf
// -----------------------------------------------------------------------------

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

float GetHeight01FromWorldY(float worldY) { return RemapLinearClamped(worldY, static_cast<float>(MIN_SURFACE_Y), static_cast<float>(MAX_SURFACE_Y), 0.0f, 1.0f); }

// -----------------------------------------------------------------------------
// continentalness
// -----------------------------------------------------------------------------

float CalculateContinentalBaseHeight(float continentalness) {

    static constexpr std::array<CurvePoint, 9> curve = {{{0.00f, -8.0f}, {0.20f, -6.0f}, {0.35f, -4.0f}, {0.40f, -2.0f}, {0.50f, 0.0f}, {0.60f, 8.0f}, {0.72f, 24.0f}, {0.86f, 52.0f}, {1.00f, 76.0f}}};

    return EvaluateCurve(curve, continentalness);
}

// -----------------------------------------------------------------------------
// ocean
// -----------------------------------------------------------------------------

float CalculateOceanDepth(float x, float z) {
    const float basinNoise = Sample01(g_noise.oceanDepth, x, z);

    const float detailNoise = g_noise.oceanDetail.GetNoise(x, z);

    const float basinDepth = basinNoise * MAX_OCEAN_DEPTH_BLOCKS;

    const float detailDepth = detailNoise * OCEAN_DETAIL_BLOCKS;

    return std::clamp(MIN_OCEAN_DEPTH_BLOCKS + basinDepth + detailDepth,

                      MIN_OCEAN_DEPTH_BLOCKS, MAX_OCEAN_DEPTH_BLOCKS);
}

float CalculateOceanHeight(float x, float z) {
    const float depth = CalculateOceanDepth(x, z);

    return static_cast<float>(SEA_LEVEL) - depth;
}

// -----------------------------------------------------------------------------
// terrain
// -----------------------------------------------------------------------------

void CalculateTerrainMasks(TerrainSample& sample) {
    const float continentalLand = SmoothStep(COAST_START, 0.78f, sample.continentalness);

    const float inland = SmoothStep(0.48f, 0.72f, sample.continentalness);

    const float ruggedness = SmoothStep(0.25f, 0.70f, 1.0f - sample.erosion);

    const float peakValue = std::abs(sample.weirdness * 2.0f - 1.0f);

    sample.ridgeMask = SmoothStep(0.35f, 0.78f, peakValue);

    sample.valleyMask = SmoothStep(0.35f, 0.85f, 1.0f - peakValue);

    sample.mountainMask = inland * ruggedness * (0.55f + 0.45f * sample.ridgeMask);

    sample.landMask = SmoothStep(COAST_START, COAST_END, sample.continentalness);

    (void)continentalLand;
}

float CalculateLandHeight(const TerrainSample& sample, float detail) {
    const float baseHeight = CalculateContinentalBaseHeight(sample.continentalness);

    const float inland = SmoothStep(0.48f, 0.72f, sample.continentalness);

    const float ruggedness = SmoothStep(0.25f, 0.70f, 1.0f - sample.erosion);

    const float peakValue = std::abs(sample.weirdness * 2.0f - 1.0f);

    const float peakStrength = SmoothStep(0.35f, 0.78f, peakValue);

    const float broadMountainHeight = sample.mountainMask * MOUNTAIN_HEIGHT_BLOCKS;

    const float valleyDepth = sample.valleyMask * inland * ruggedness * VALLEY_DEPTH_BLOCKS;

    const float climateVariation = (sample.moisture - 0.5f) * 4.0f + (0.5f - sample.erosion) * 4.0f;

    const float localDetail = detail * TERRAIN_DETAIL_BLOCKS * (0.35f + 0.65f * inland);

    const float peakAccent = peakStrength * ruggedness * inland * 20.0f;

    return static_cast<float>(SEA_LEVEL) + baseHeight + broadMountainHeight + peakAccent - valleyDepth + climateVariation + localDetail;
}

float CalculateSurfaceHeight(const TerrainSample& sample, float x, float z, float detail) {
    const float landHeight = CalculateLandHeight(sample, detail);

    const float oceanHeight = CalculateOceanHeight(x, z);

    const float oceanBlend = InverseSmoothStep(COAST_START, COAST_END, sample.continentalness);

    float surfaceY = Lerp(landHeight, oceanHeight, oceanBlend);

    if (sample.continentalness >= COAST_END) { surfaceY = landHeight; }

    if (sample.continentalness <= COAST_START) { surfaceY = oceanHeight; }

    return std::clamp(surfaceY, static_cast<float>(MIN_SURFACE_Y), static_cast<float>(MAX_SURFACE_Y));
}

// -----------------------------------------------------------------------------
// biomes
// -----------------------------------------------------------------------------

struct BiomeClimateProfile {
    const Biome* biome;

    float temperature;
    float moisture;

    float temperatureWeight;
    float moistureWeight;
};

float ClimateDistance(const TerrainSample& sample, const BiomeClimateProfile& profile) {
    const float temperatureDelta = sample.temperature - profile.temperature;

    const float moistureDelta = sample.moisture - profile.moisture;

    return temperatureDelta * temperatureDelta * profile.temperatureWeight + moistureDelta * moistureDelta * profile.moistureWeight;
}

bool IsNearSea(const TerrainSample& sample) {
    constexpr float BEACH_BELOW_BLOCKS = 4.0f;
    constexpr float BEACH_ABOVE_BLOCKS = 6.0f;

    return sample.surfaceY >= static_cast<float>(SEA_LEVEL) - BEACH_BELOW_BLOCKS && sample.surfaceY <= static_cast<float>(SEA_LEVEL) + BEACH_ABOVE_BLOCKS;
}

const Biome* SelectLandBiome(const TerrainSample& sample) {
    if (sample.continentalness <= COAST_END && IsNearSea(sample)) { return &Biomes::Beach; }

    if (sample.mountainMask > 0.34f) {
        if (sample.temperature < 0.28f || sample.surfaceY > 220.0f) { return &Biomes::SnowPlains; }

        return &Biomes::Mountains;
    }

    if (sample.temperature > 0.66f && sample.moisture < 0.32f && sample.ridgeMask > 0.48f && sample.erosion < 0.55f) { return &Biomes::Badlands; }

    static constexpr std::array<BiomeClimateProfile, 5> profiles = {{{&Biomes::SnowPlains, 0.12f, 0.48f, 1.5f, 0.65f},
                                                                     {&Biomes::Taiga, 0.30f, 0.72f, 1.0f, 1.0f},
                                                                     {&Biomes::Desert, 0.82f, 0.18f, 1.0f, 1.25f},
                                                                     {&Biomes::Forest, 0.55f, 0.76f, 0.85f, 1.0f},
                                                                     {&Biomes::Plains, 0.50f, 0.45f, 0.70f, 0.70f}}};

    const Biome* selectedBiome = &Biomes::Plains;
    float bestDistance = std::numeric_limits<float>::max();

    for (const BiomeClimateProfile& profile : profiles) {
        const float distance = ClimateDistance(sample, profile);

        if (distance < bestDistance) {
            bestDistance = distance;
            selectedBiome = profile.biome;
        }
    }

    return selectedBiome;
}

// -----------------------------------------------------------------------------
// public sampling
// -----------------------------------------------------------------------------

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

    ConfigureNoise(g_noise.oceanDetail, HashSeed(seed, 9), OCEAN_DETAIL_FREQUENCY, 3);

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

    CalculateTerrainMasks(sample);

    sample.surfaceY = CalculateSurfaceHeight(sample, x, z, detail);

    sample.height01 = GetHeight01FromWorldY(sample.surfaceY);

    sample.ocean = sample.surfaceY < static_cast<float>(SEA_LEVEL) - 0.5f;

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

    const float clampedSurface = std::clamp(sample.surfaceY, static_cast<float>(MIN_SURFACE_Y), static_cast<float>(MAX_SURFACE_Y));

    return static_cast<int32_t>(std::lround(clampedSurface));
}

} // namespace TerrainGeneration
