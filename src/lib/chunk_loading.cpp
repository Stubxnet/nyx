#include <iostream>
#include <fstream>
#include "lib/Chunk.hpp"
#include "pb/chunk.pb.h"
#include "World.hpp"
#include "world.pb.h"

bool ReadChunkFile(const std::string& file, Chunk& chunk) {
    ChunkProto chunkProto;
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        std::cerr << "Could not open file: " << file << std::endl;
        return false;
    }
    if (!chunkProto.ParseFromIstream(&input)) {
        std::cerr << "Failed to parse chunk data." << std::endl;
        return false;
    }

    chunk.setChunkId(chunkProto.chunk_id());
    chunk.setDimensionId(chunkProto.dimension_id());
    chunk.setBiome(chunkProto.biome());
    chunk.setSurBiome(chunkProto.surbiome());

    for (const auto& blockProto : chunkProto.blocks()) {
        Block block;
        block.setPositionX(blockProto.position_x());
        block.setPositionY(blockProto.position_y());
        block.setPositionZ(blockProto.position_z());
        block.setId(blockProto.id());
        block.setTexture(blockProto.texture());
        block.setModelId(blockProto.modelid());
        block.setBreakable(blockProto.breakable());
        block.setResistance(blockProto.resistance());
        block.setNoTexture(blockProto.notexture());
        block.setHitbox(blockProto.hitbox());
        block.setMiningSound(blockProto.miningsound());
        block.setBreakSound(blockProto.breaksound());
        block.setWalkSound(blockProto.walksound());
        block.setNatural(blockProto.natural());
        chunk.getBlocks().push_back(block);
    }

    return true;
}

bool ModifyChunkFile(const Chunk& chunk, const std::string& file) {
    ChunkProto chunkProto;
    chunkProto.set_chunk_id(chunk.getChunkId());
    chunkProto.set_dimension_id(chunk.getDimensionId());
    chunkProto.set_biome(chunk.getBiome());
    chunkProto.set_surbiome(chunk.getSurBiome());

    for (const auto& block : chunk.getBlocks()) {
        BlockProto* blockProto = chunkProto.add_blocks();
        blockProto->set_position_x(block.getPositionX());
        blockProto->set_position_y(block.getPositionY());
        blockProto->set_position_z(block.getPositionZ());
        blockProto->set_id(block.getId());
        blockProto->set_texture(block.getTexture());
        blockProto->set_modelid(block.getModelId());
        blockProto->set_breakable(block.isBreakable());
        blockProto->set_resistance(block.getResistance());
        blockProto->set_notexture(block.hasNoTexture());
        blockProto->set_hitbox(block.getHitbox());
        blockProto->set_miningsound(block.getMiningSound());
        blockProto->set_breaksound(block.getBreakSound());
        blockProto->set_walksound(block.getWalkSound());
        blockProto->set_natural(block.isNatural());
    }

    std::ofstream output(file, std::ios::binary);
    if (!chunkProto.SerializeToOstream(&output)) {
        std::cerr << "Failed to write chunk data." << std::endl;
        return false;
    }
    return true;
}

bool ReadWorldFile(const std::string& file, World& world) {
    WorldProto worldProto;
    std::ifstream input(file, std::ios::binary);
    if (!input) {
        std::cerr << "Could not open file: " << file << std::endl;
        return false;
    }
    if (!worldProto.ParseFromIstream(&input)) {
        std::cerr << "Failed to parse world data." << std::endl;
        return false;
    }

    world.setName(worldProto.name());
    world.setSeed(worldProto.seed());
    world.setPresets(worldProto.presets());
    world.setHosted(worldProto.hosted());

    for (const auto& chunkProto : worldProto.chunks()) {
        Chunk chunk;
        chunk.setChunkId(chunkProto.chunk_id());
        chunk.setFile(chunkProto.file());
        world.getChunks().push_back(chunk);
    }

    return true;
}

bool ModifyWorldFile(const World& world, const std::string& file) {
    WorldProto worldProto;
    worldProto.set_name(world.getName());
    worldProto.set_seed(world.getSeed());
    worldProto.set_presets(world.getPresets());
    worldProto.set_hosted(world.isHosted());

    for (const auto& chunk : world.getChunks()) {
        ChunkProto* chunkProto = worldProto.add_chunks();
        chunkProto->set_chunk_id(chunk.getChunkId());
        chunkProto->set_file(chunk.getFile());
    }

    std::ofstream output(file, std::ios::binary);
    if (!worldProto.SerializeToOstream(&output)) {
        std::cerr << "Failed to write world data." << std::endl;
        return false;
    }
    return true;
}