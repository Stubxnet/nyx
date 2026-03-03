#include "../lib/Chunk.hpp"
#include "../lib/Block.hpp"

Chunk generateVoidChunk(int chunk_size, int id, const std::string& biome, const std::string& surbiome) {
    Chunk chunk(0.0f, 0.0f, 0.0f, biome, surbiome);

    for (int x = 0; x < chunk_size; ++x) {
        for (int y = 0; y < chunk_size; ++y) {
            for (int z = 0; z < chunk_size; ++z) {
                Block block(x, y, z, id);
                chunk.addBlock(block);
            }
        }
    }

    return chunk;
}