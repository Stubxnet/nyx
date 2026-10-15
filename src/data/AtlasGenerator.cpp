#pragma once

#include <raylib.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <array>
#include <cstdint>
#include <stdexcept>
#include <limits>
#include <cmath>

#include <nlohmann/json.hpp>

#include "../data/lib/BlocksDefaults.hpp"

#include "../utils/utils.cpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

struct ImageAutoUnload {
    Image& image;

    explicit ImageAutoUnload(Image& value)
        : image(value) {}

    ~ImageAutoUnload() {
        if (image.data != nullptr) { UnloadImage(image); }
    }

    ImageAutoUnload(const ImageAutoUnload&) = delete;
    ImageAutoUnload& operator=(const ImageAutoUnload&) = delete;
};

static inline int ceilSqrtInt(int x) {
    if (x <= 0) return 0;

    int r = static_cast<int>(std::sqrt(static_cast<double>(x)));
    while (static_cast<long long>(r) * r < x) ++r;
    while (r > 0 && static_cast<long long>(r - 1) * (r - 1) >= x) --r;

    return r;
}

static inline Rectangle normalizedUVFromIndex(int idx, int atlasCols, int atlasRows, int atlasW, int atlasH) {
    if (idx < 0 || atlasCols <= 0 || atlasRows <= 0 || atlasW <= 0 || atlasH <= 0 || TILE <= 0) { throw std::runtime_error("Invalid atlas dimensions or texture index."); }

    const int col = idx % atlasCols;
    const int row = idx / atlasCols;

    if (row >= atlasRows) { throw std::runtime_error("Texture index is outside the atlas grid."); }

    const float x = static_cast<float>(col * TILE) / static_cast<float>(atlasW);
    const float yTop = static_cast<float>(row * TILE) / static_cast<float>(atlasH);
    const float w = static_cast<float>(TILE) / static_cast<float>(atlasW);
    const float h = static_cast<float>(TILE) / static_cast<float>(atlasH);

    return Rectangle{x, 1.0f - (yTop + h), w, h};
}

static inline uint64_t fnv1a64(const uint8_t* data, size_t n) {
    if (data == nullptr && n != 0) { throw std::runtime_error("Cannot hash a null pixel buffer."); }

    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; ++i) {
        h ^= static_cast<uint64_t>(data[i]);
        h *= 1099511628211ull;
    }
    return h;
}

static inline uint64_t hashImagePixels(Image& img) {
    if (img.data == nullptr || img.width <= 0 || img.height <= 0) { throw std::runtime_error("Cannot hash an invalid image."); }

    if (img.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) { ImageFormat(&img, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8); }

    if (img.data == nullptr || img.format != PIXELFORMAT_UNCOMPRESSED_R8G8B8A8) { throw std::runtime_error("Failed to convert image to RGBA8."); }

    const int dataSize = GetPixelDataSize(img.width, img.height, img.format);
    if (dataSize <= 0) { throw std::runtime_error("Raylib returned an invalid image data size."); }

    const uint64_t expectedSize = static_cast<uint64_t>(img.width) * static_cast<uint64_t>(img.height) * 4ull;

    if (expectedSize != static_cast<uint64_t>(dataSize)) { throw std::runtime_error("RGBA image size does not match its dimensions."); }

    return fnv1a64(static_cast<const uint8_t*>(img.data), static_cast<size_t>(dataSize));
}

static inline void writeAtlasUVJson(const BlocksDefaults& blocksDefaults, const fs::path& outPath) {
    if (outPath.empty()) { throw std::runtime_error("Atlas UV output path is empty."); }

    json j;
    j["version"] = 1;
    j["atlasCols"] = blocksDefaults.atlasCols;
    j["atlasRows"] = blocksDefaults.atlasRows;
    j["atlasW"] = blocksDefaults.atlasW;
    j["atlasH"] = blocksDefaults.atlasH;

    json blocksJson = json::object();

    for (const auto& [id, block] : blocksDefaults.loaded.blocks) {
        json facesJson = json::object();

        for (int fi = 0; fi < 6; ++fi) {
            const Rectangle& uv = block.faces[fi].uv;

            if (!std::isfinite(uv.x) || !std::isfinite(uv.y) || !std::isfinite(uv.width) || !std::isfinite(uv.height) || uv.width <= 0.0f || uv.height <= 0.0f) {
                throw std::runtime_error("Cannot save invalid atlas UV coordinates.");
            }

            facesJson[FaceNames[fi]] = {{"x", uv.x}, {"y", uv.y}, {"w", uv.width}, {"h", uv.height}};
        }

        blocksJson[std::to_string(id)] = std::move(facesJson);
    }

    j["blocks"] = std::move(blocksJson);

    const fs::path parent = outPath.parent_path();
    if (!parent.empty()) { fs::create_directories(parent); }

    std::ofstream file(outPath);
    if (!file.is_open()) { throw std::runtime_error("Cannot write UV JSON: " + outPath.string()); }

    file << j.dump(2);
    file.flush();

    if (!file.good()) { throw std::runtime_error("Failed while writing UV JSON: " + outPath.string()); }
}

static inline bool loadAtlasUVJson(BlocksDefaults& blocksDefaults, const fs::path& uvPath) {
    try {
        if (uvPath.empty() || !fs::exists(uvPath) || !fs::is_regular_file(uvPath)) { return false; }

        const std::string raw = readTextFile(uvPath);
        if (raw.find_first_not_of(" \t\r\n") == std::string::npos) { return false; }

        const json j = json::parse(raw);
        if (!j.is_object() || !j.contains("blocks") || !j["blocks"].is_object()) { return false; }

        const int atlasCols = j.at("atlasCols").get<int>();
        const int atlasRows = j.at("atlasRows").get<int>();
        const int atlasW = j.at("atlasW").get<int>();
        const int atlasH = j.at("atlasH").get<int>();

        if (atlasCols <= 0 || atlasRows <= 0 || atlasW <= 0 || atlasH <= 0 || atlasCols > std::numeric_limits<uint16_t>::max() || atlasRows > std::numeric_limits<uint16_t>::max()) { return false; }

        std::vector<std::pair<uint16_t, std::array<Rectangle, 6>>> pending;

        for (uint16_t id : blocksDefaults.sortedIds) {
            auto blockIt = j["blocks"].find(std::to_string(id));
            if (blockIt == j["blocks"].end() || !blockIt->is_object()) { return false; }

            std::array<Rectangle, 6> faces{};

            for (int fi = 0; fi < 6; ++fi) {
                const char* key = FaceNames[fi];
                if (!blockIt->contains(key) || !(*blockIt)[key].is_object()) { return false; }

                const json& uvJson = (*blockIt)[key];
                Rectangle uv{uvJson.at("x").get<float>(), uvJson.at("y").get<float>(), uvJson.at("w").get<float>(), uvJson.at("h").get<float>()};

                if (!std::isfinite(uv.x) || !std::isfinite(uv.y) || !std::isfinite(uv.width) || !std::isfinite(uv.height) || uv.width <= 0.0f || uv.height <= 0.0f) { return false; }

                faces[fi] = uv;
            }

            pending.emplace_back(id, faces);
        }

        for (const auto& [id, faces] : pending) {
            auto blockIt = blocksDefaults.loaded.blocks.find(id);
            if (blockIt == blocksDefaults.loaded.blocks.end()) { return false; }

            for (int fi = 0; fi < 6; ++fi) { blockIt->second.faces[fi].uv = faces[fi]; }
        }

        blocksDefaults.atlasCols = static_cast<uint16_t>(atlasCols);
        blocksDefaults.atlasRows = static_cast<uint16_t>(atlasRows);
        blocksDefaults.atlasW = atlasW;
        blocksDefaults.atlasH = atlasH;

        return true;
    } catch (...) { return false; }
}

static inline void saveAtlasPNG(const BlocksDefaults& blocksDefaults, const fs::path& outPngPath) {
    if (outPngPath.empty()) { throw std::runtime_error("Atlas PNG output path is empty."); }

    if (!blocksDefaults.atlasReady || blocksDefaults.atlasRT.id == 0 || blocksDefaults.atlasRT.texture.id == 0) { throw std::runtime_error("Cannot save an atlas that is not ready."); }

    const fs::path parent = outPngPath.parent_path();
    if (!parent.empty()) { fs::create_directories(parent); }

    Image img = LoadImageFromTexture(blocksDefaults.atlasRT.texture);
    ImageAutoUnload imageGuard(img);

    if (img.data == nullptr || img.width <= 0 || img.height <= 0) { throw std::runtime_error("Failed to convert atlas texture to an image."); }

    if (!ExportImage(img, outPngPath.string().c_str())) { throw std::runtime_error("Failed to export atlas PNG: " + outPngPath.string()); }
}

static inline void buildAtlasForBlocksImpl(BlocksDefaults& blocksDefaults, const fs::path& texturesDir, const fs::path& atlasPngOut, const fs::path& atlasUvOut) {
    if (TILE <= 0) { throw std::runtime_error("TILE must be greater than zero."); }

    unloadAtlas(blocksDefaults);

    try {
        constexpr std::array<int, 6> faceOrder = {TOP, BOTTOM, EAST, WEST, NORTH, SOUTH};

        std::unordered_map<uint64_t, int> hashToAtlasIndex;
        std::unordered_map<std::string, int> nameToAtlasIndex;
        std::vector<std::string> orderedAtlasTextureNames;

        for (uint16_t id : blocksDefaults.sortedIds) {
            auto blockIt = blocksDefaults.loaded.blocks.find(id);
            if (blockIt == blocksDefaults.loaded.blocks.end()) { throw std::runtime_error("Block ID is missing from loaded blocks: " + std::to_string(id)); }

            BlockReference& block = blockIt->second;

            for (int fi : faceOrder) {
                const std::string& texNameRaw = block.faces[fi].textureName;
                if (texNameRaw.empty()) continue;

                if (nameToAtlasIndex.find(texNameRaw) != nameToAtlasIndex.end()) { continue; }

                fs::path texPath = texturesDir / texNameRaw;
                if (texPath.extension().empty()) { texPath += ".png"; }

                Image img = loadImageOrFallback(texPath);
                ImageAutoUnload imageGuard(img);

                if (img.data == nullptr || img.width <= 0 || img.height <= 0) { throw std::runtime_error("Failed to load texture image: " + texPath.string()); }

                if (img.width != TILE || img.height != TILE) {
                    throw std::runtime_error("Texture has unexpected dimensions after ensureTileSize: " + texPath.string() + " (" + std::to_string(img.width) + "x" + std::to_string(img.height) +
                                             ", expected " + std::to_string(TILE) + "x" + std::to_string(TILE) + ")");
                }

                const uint64_t hash = hashImagePixels(img);

                int atlasIdx;
                auto hit = hashToAtlasIndex.find(hash);

                if (hit == hashToAtlasIndex.end()) {
                    atlasIdx = static_cast<int>(orderedAtlasTextureNames.size());
                    hashToAtlasIndex.emplace(hash, atlasIdx);
                    orderedAtlasTextureNames.push_back(texNameRaw);
                } else {
                    atlasIdx = hit->second;
                }

                nameToAtlasIndex.emplace(texNameRaw, atlasIdx);
            }
        }

        if (orderedAtlasTextureNames.size() > static_cast<size_t>(std::numeric_limits<int>::max())) { throw std::runtime_error("Too many atlas textures."); }

        const int N = static_cast<int>(orderedAtlasTextureNames.size());

        if (N == 0) {
            blocksDefaults.atlasCols = 1;
            blocksDefaults.atlasRows = 1;
            blocksDefaults.atlasW = TILE;
            blocksDefaults.atlasH = TILE;

            blocksDefaults.atlasRT = LoadRenderTexture(TILE, TILE);
            if (blocksDefaults.atlasRT.id == 0 || blocksDefaults.atlasRT.texture.id == 0) {
                blocksDefaults.atlasRT = RenderTexture2D{};
                throw std::runtime_error("Failed to create dummy atlas.");
            }

            blocksDefaults.atlasTex = blocksDefaults.atlasRT.texture;
            blocksDefaults.atlasReady = true;

            for (uint16_t id : blocksDefaults.sortedIds) {
                auto blockIt = blocksDefaults.loaded.blocks.find(id);
                if (blockIt == blocksDefaults.loaded.blocks.end()) { throw std::runtime_error("Block ID is missing from loaded blocks: " + std::to_string(id)); }

                for (int fi : faceOrder) { blockIt->second.faces[fi].uv = normalizedUVFromIndex(0, 1, 1, TILE, TILE); }
            }

            saveAtlasPNG(blocksDefaults, atlasPngOut);
            writeAtlasUVJson(blocksDefaults, atlasUvOut);

            std::cout << "Atlas built: unique textures=0 (dummy 1 tile)\n";
            return;
        }

        const int atlasCols = ceilSqrtInt(N);
        const int atlasRows = (N + atlasCols - 1) / atlasCols;

        const long long atlasW64 = static_cast<long long>(atlasCols) * TILE;
        const long long atlasH64 = static_cast<long long>(atlasRows) * TILE;

        if (atlasW64 <= 0 || atlasH64 <= 0 || atlasW64 > std::numeric_limits<int>::max() || atlasH64 > std::numeric_limits<int>::max() || atlasCols > std::numeric_limits<uint16_t>::max() ||
            atlasRows > std::numeric_limits<uint16_t>::max()) {
            throw std::runtime_error("Atlas dimensions exceed supported limits.");
        }

        const int atlasW = static_cast<int>(atlasW64);
        const int atlasH = static_cast<int>(atlasH64);

        blocksDefaults.atlasCols = static_cast<uint16_t>(atlasCols);
        blocksDefaults.atlasRows = static_cast<uint16_t>(atlasRows);
        blocksDefaults.atlasW = atlasW;
        blocksDefaults.atlasH = atlasH;

        blocksDefaults.atlasRT = LoadRenderTexture(atlasW, atlasH);
        if (blocksDefaults.atlasRT.id == 0 || blocksDefaults.atlasRT.texture.id == 0) {
            blocksDefaults.atlasRT = RenderTexture2D{};
            throw std::runtime_error("Failed to create atlas render texture.");
        }

        blocksDefaults.atlasTex = blocksDefaults.atlasRT.texture;
        blocksDefaults.atlasReady = true;

        BeginTextureMode(blocksDefaults.atlasRT);
        try {
            ClearBackground(Color{0, 0, 0, 0});

            for (int i = 0; i < N; ++i) {
                const std::string& texNameRaw = orderedAtlasTextureNames[static_cast<size_t>(i)];

                fs::path path = texturesDir / texNameRaw;
                if (path.extension().empty()) { path += ".png"; }

                Texture2D tex = loadTextureOrFallback(path);
                if (tex.id == 0 || tex.width <= 0 || tex.height <= 0) {
                    if (tex.id != 0) UnloadTexture(tex);
                    throw std::runtime_error("Failed to load texture: " + path.string());
                }

                try {
                    blocksDefaults.atlasTextures.push_back(tex);
                } catch (...) {
                    UnloadTexture(tex);
                    throw;
                }

                const int col = i % atlasCols;
                const int row = i / atlasCols;

                const Rectangle src{0.0f, 0.0f, static_cast<float>(tex.width), static_cast<float>(tex.height)};
                const Rectangle dst{static_cast<float>(col * TILE), static_cast<float>(row * TILE), static_cast<float>(TILE), static_cast<float>(TILE)};

                DrawTexturePro(tex, src, dst, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
            }
        } catch (...) {
            EndTextureMode();
            throw;
        }
        EndTextureMode();

        for (uint16_t id : blocksDefaults.sortedIds) {
            auto blockIt = blocksDefaults.loaded.blocks.find(id);
            if (blockIt == blocksDefaults.loaded.blocks.end()) { throw std::runtime_error("Block ID is missing from loaded blocks: " + std::to_string(id)); }

            BlockReference& block = blockIt->second;

            for (int fi : faceOrder) {
                const std::string& texNameRaw = block.faces[fi].textureName;
                if (texNameRaw.empty()) continue;

                auto it = nameToAtlasIndex.find(texNameRaw);
                if (it == nameToAtlasIndex.end()) { throw std::runtime_error("Missing atlas index for texture: " + texNameRaw); }

                block.faces[fi].uv = normalizedUVFromIndex(it->second, atlasCols, atlasRows, atlasW, atlasH);
            }
        }

        saveAtlasPNG(blocksDefaults, atlasPngOut);
        writeAtlasUVJson(blocksDefaults, atlasUvOut);

        std::cout << "Atlas built: unique textures=" << N << " atlasCols=" << atlasCols << " atlasRows=" << atlasRows << " atlasSize=" << atlasW << "x" << atlasH << "\n";
    } catch (...) {
        unloadAtlas(blocksDefaults);
        throw;
    }
}

static inline bool tryLoadSavedAtlasImpl(BlocksDefaults& blocksDefaults, const fs::path& atlasPngPath, const fs::path& atlasUvPath) {
    try {
        if (!fs::exists(atlasPngPath) || !fs::is_regular_file(atlasPngPath) || !fs::exists(atlasUvPath) || !fs::is_regular_file(atlasUvPath)) { return false; }

        unloadAtlas(blocksDefaults);

        Image img = LoadImage(atlasPngPath.string().c_str());
        ImageAutoUnload imageGuard(img);

        if (img.data == nullptr || img.width <= 0 || img.height <= 0) { return false; }

        const int imageW = img.width;
        const int imageH = img.height;

        Texture2D texture = LoadTextureFromImage(img);
        if (texture.id == 0) { return false; }

        blocksDefaults.atlasW = imageW;
        blocksDefaults.atlasH = imageH;
        blocksDefaults.atlasTex = texture;
        blocksDefaults.atlasReady = true;
        blocksDefaults.atlasRT = RenderTexture2D{};

        const bool uvLoaded = loadAtlasUVJson(blocksDefaults, atlasUvPath);
        if (!uvLoaded || blocksDefaults.atlasW != imageW || blocksDefaults.atlasH != imageH) {
            unloadAtlas(blocksDefaults);
            return false;
        }

        return true;
    } catch (...) {
        unloadAtlas(blocksDefaults);
        return false;
    }
}

void buildAtlasForBlocks(BlocksDefaults& blocksDefaults, const fs::path& texturesDir, const fs::path& atlasPngOut, const fs::path& atlasUvOut) {
    buildAtlasForBlocksImpl(blocksDefaults, texturesDir, atlasPngOut, atlasUvOut);
}

bool tryLoadSavedAtlas(BlocksDefaults& blocksDefaults, const fs::path& atlasPngPath, const fs::path& atlasUvPath) { return tryLoadSavedAtlasImpl(blocksDefaults, atlasPngPath, atlasUvPath); }
