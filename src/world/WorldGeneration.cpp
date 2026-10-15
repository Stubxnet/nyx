#include "TerrainGeneration.cpp"

#include "../core/constants.hpp"
#include "../world/lib/Chunk.hpp"
#include "../world/lib/VerticalChunk.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>

void ConfigureWorldGeneration(uint32_t seed) { TerrainGeneration::Configure(seed); }

std::shared_ptr<VerticalChunk> GenerateChunkHeightmap(int32_t cx, int32_t cz, uint32_t seed) {
    TerrainGeneration::EnsureInitialized(seed);

    auto result = std::make_shared<VerticalChunk>(cx, cz);

    for (int localX = 0; localX < CHUNK_SIZE; ++localX) {
        for (int localZ = 0; localZ < CHUNK_SIZE; ++localZ) {
            const int64_t worldX = static_cast<int64_t>(cx) * static_cast<int64_t>(CHUNK_SIZE) + static_cast<int64_t>(localX);

            const int64_t worldZ = static_cast<int64_t>(cz) * static_cast<int64_t>(CHUNK_SIZE) + static_cast<int64_t>(localZ);

            const TerrainGeneration::TerrainSample sample = TerrainGeneration::Sample(worldX, worldZ);

            const int32_t surfaceY = TerrainGeneration::ComputeSurfaceY(sample, worldX, worldZ);

            auto& cell = result->GetCell(localX, localZ);

            cell = {surfaceY, sample.temperature, sample.moisture, sample.erosion, sample.continentalness, *sample.biome};
        }
    }

    return result;
}

std::shared_ptr<Chunk> GenerateChunkBlocks(const VerticalChunk& verticalChunk, int32_t cy) {
    const int32_t cx = verticalChunk.GetChunkX();

    const int32_t cz = verticalChunk.GetChunkZ();

    const int64_t baseY = static_cast<int64_t>(cy) * static_cast<int64_t>(CHUNK_SIZE);

    auto chunk = std::make_shared<Chunk>(cx, cy, cz);

    for (int x = 0; x < CHUNK_SIZE; ++x) {
        for (int z = 0; z < CHUNK_SIZE; ++z) {
            const auto& cell = verticalChunk.GetCell(x, z);

            const Biome& biome = cell.biome;

            const int64_t surfaceY = static_cast<int64_t>(cell.surfaceY);

            const int topBlocks = std::max(0, static_cast<int>(biome.topBlocks));

            const int soilBlocks = std::max(0, static_cast<int>(biome.soilBlocks));

            const int64_t exclusiveSolidTop = surfaceY + 1;

            const bool ocean = biome.id == BiomeId::Ocean;

            if (baseY >= exclusiveSolidTop) {
                if (ocean && baseY <= static_cast<int64_t>(SEA_LEVEL)) {
                    for (int y = 0; y < CHUNK_SIZE; ++y) {
                        const int64_t worldY = baseY + static_cast<int64_t>(y);

                        // if (worldY <= SEA_LEVEL) { chunk->SetBlockId(x, y, z, BlockId{2}); }
                        // will be used when water will be added
                    }
                }

                continue;
            }

            for (int y = 0; y < CHUNK_SIZE; ++y) {
                const int64_t worldY = baseY + static_cast<int64_t>(y);

                if (worldY <= surfaceY) {
                    const int64_t depth = surfaceY - worldY;

                    BlockId block = biome.underground;

                    if (depth < topBlocks) {
                        block = biome.top;
                    } else if (depth < topBlocks + soilBlocks) {
                        block = biome.soil;
                    }

                    if (block != BlockId{0}) { chunk->SetBlockId(x, y, z, block); }

                    continue;
                }
                // if (ocean && worldY <= SEA_LEVEL) { chunk->SetBlockId(x, y, z, BlockId{2}); }
                // same comment as up there
            }
        }
    }

    chunk->UnmarkAsDirty();
    chunk->SetState(ChunkState::Generated);

    return chunk;
}
