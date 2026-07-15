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

#include <nlohmann/json.hpp>

#include "../lib/BlockDefaults.hpp"
#include "../utils/IO_Utils.cpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

static inline int ceilSqrtInt(int x) {
    int r = 0;
    while (r * r < x) ++r;
    return r;
}

static inline Rectangle normalizedUVFromIndex(int idx, int atlasCols, int atlasRows, int atlasW, int atlasH) {
    int col = idx % atlasCols;
    int row = idx / atlasCols;

    float x = (float)(col * TILE_SIZE) / (float)atlasW;
    float yTop = (float)(row * TILE_SIZE) / (float)atlasH;

    float w = (float)TILE_SIZE / (float)atlasW;
    float h = (float)TILE_SIZE / (float)atlasH;

    float y = 1.0f - (yTop + h);
    return Rectangle{ x, y, w, h };
}

static inline uint64_t fnv1a64(const uint8_t* data, size_t n) {
    uint64_t h = 1469598103934665603ull;
    for (size_t i = 0; i < n; ++i) {
        h ^= (uint64_t)data[i];
        h *= 1099511628211ull;
    }
    return h;
}

static inline uint64_t hashImagePixels(const Image& img) {
    const uint8_t* bytes = (const uint8_t*)img.data;
    size_t n = (size_t)img.width * (size_t)img.height * 4ull;
    return fnv1a64(bytes, n);
}

static inline void writeAtlasUVJson(const BlocksDefaults& blocksDefaults, const fs::path& outPath) {
    json j;
    j["version"] = 1;
    j["atlasCols"] = blocksDefaults.atlasCols;
    j["atlasRows"] = blocksDefaults.atlasRows;
    j["atlasW"] = blocksDefaults.atlasW;
    j["atlasH"] = blocksDefaults.atlasH;

    json blocksJson = json::object();
    for (auto& [id, b] : blocksDefaults.loaded.blocks) {
        json fb = json::object();
        for (int fi = 0; fi < 6; ++fi) {
            const Rectangle& uv = b.faces[fi].uv;
            fb[FaceNames[fi]] = {
                {"x", uv.x},
                {"y", uv.y},
                {"w", uv.width},
                {"h", uv.height}
            };
        }
        blocksJson[std::to_string(id)] = fb;
    }
    j["blocks"] = blocksJson;

    fs::create_directories(outPath.parent_path());
    std::ofstream f(outPath);
    if (!f) throw std::runtime_error("Cannot write UV json: " + outPath.string());
    f << j.dump(2);
}

static inline bool loadAtlasUVJson(BlocksDefaults& blocksDefaults, const fs::path& uvPath) {
    if (!fs::exists(uvPath) || !fs::is_regular_file(uvPath)) return false;

    std::string raw = IOutils::readTextFile(uvPath);
    if (raw.find_first_not_of(" \t\r\n") == std::string::npos) return false;

    json j;
    try {
        j = json::parse(raw);
    } catch (...) {
        return false;
    }

    if (!j.is_object()) return false;

    int atlasCols = j.value("atlasCols", 0);
    int atlasRows = j.value("atlasRows", 0);
    int atlasW = j.value("atlasW", 0);
    int atlasH = j.value("atlasH", 0);
    if (atlasCols <= 0 || atlasRows <= 0 || atlasW <= 0 || atlasH <= 0) return false;

    if (!j.contains("blocks") || !j["blocks"].is_object()) return false;

    for (uint16_t id : blocksDefaults.sortedIds) {
        auto it = j["blocks"].find(std::to_string(id));
        if (it == j["blocks"].end() || !it->is_object()) return false;

        const json& bj = *it;

        for (int fi = 0; fi < 6; ++fi) {
            const char* key = FaceNames[fi];
            if (!bj.contains(key) || !bj[key].is_object()) return false;

            const json& u = bj[key];
            if (!u.contains("x") || !u.contains("y") || !u.contains("w") || !u.contains("h")) return false;

            blocksDefaults.loaded.blocks[id].faces[fi].uv = Rectangle{
                u.at("x").get<float>(),
                u.at("y").get<float>(),
                u.at("w").get<float>(),
                u.at("h").get<float>()
            };
        }
    }

    blocksDefaults.atlasCols = (uint16_t)atlasCols;
    blocksDefaults.atlasRows = (uint16_t)atlasRows;
    blocksDefaults.atlasW = atlasW;
    blocksDefaults.atlasH = atlasH;
    return true;
}

static inline void saveAtlasPNG(const BlocksDefaults& blocksDefaults, const fs::path& outPngPath) {
    if (!blocksDefaults.atlasReady || blocksDefaults.atlasRT.id == 0) return;

    fs::create_directories(outPngPath.parent_path());

    Image img = LoadImageFromTexture(blocksDefaults.atlasRT.texture);
    if (img.width <= 0 || img.height <= 0) {
        UnloadImage(img);
        throw std::runtime_error("Failed to convert atlas texture to image.");
    }

    bool ok = ExportImage(img, outPngPath.string().c_str());
    UnloadImage(img);

    if (!ok) throw std::runtime_error("Failed to export atlas PNG: " + outPngPath.string());
}

static inline void buildAtlasForBlocksImpl(
    BlocksDefaults& blocksDefaults,
    const fs::path& texturesDir,
    const fs::path& atlasPngOut,
    const fs::path& atlasUvOut
) {
    unloadAtlas(blocksDefaults);

    constexpr std::array<int, 6> faceOrder = { TOP, BOTTOM, EAST, WEST, NORTH, SOUTH };

    std::unordered_map<uint64_t, int> hashToAtlasIndex;
    std::unordered_map<std::string, int> nameToAtlasIndex;
    std::vector<std::string> orderedAtlasTextureNames;

    for (uint16_t id : blocksDefaults.sortedIds) {
        auto it = blocksDefaults.loaded.blocks.find(id);
        if (it == blocksDefaults.loaded.blocks.end()) continue;
        BlockReference& b = it->second;

        for (int fi : faceOrder) {
            const std::string& texNameRaw = b.faces[fi].textureName;
            if (texNameRaw.empty()) continue;

            // IMPORTANT: on stocke la clé "telle quelle" pour garder la logique actuelle
            // (mais le chargement doit gérer l'extension).
            if (nameToAtlasIndex.find(texNameRaw) != nameToAtlasIndex.end()) continue;

            // ✅ CORRECTIF: si textureName n'a pas d'extension => ajouter ".png"
            fs::path texPath = texturesDir / texNameRaw;
            if (texPath.extension().empty()) {
                texPath += ".png";
            }

            Image img = IOutils::loadImageOrFallback(texPath);
            if (img.width <= 0 || img.height <= 0) {
                throw std::runtime_error("Failed to load texture image: " + texPath.string());
            }

            uint64_t h = hashImagePixels(img);
            UnloadImage(img);

            int atlasIdx;
            auto hit = hashToAtlasIndex.find(h);
            if (hit == hashToAtlasIndex.end()) {
                atlasIdx = (int)orderedAtlasTextureNames.size();
                hashToAtlasIndex.emplace(h, atlasIdx);
                orderedAtlasTextureNames.push_back(texNameRaw);
            } else {
                atlasIdx = hit->second;
            }

            nameToAtlasIndex.emplace(texNameRaw, atlasIdx);
        }
    }

    const int N = (int)orderedAtlasTextureNames.size();

    if (N == 0) {
        blocksDefaults.atlasCols = 1;
        blocksDefaults.atlasRows = 1;
        blocksDefaults.atlasW = TILE_SIZE;
        blocksDefaults.atlasH = TILE_SIZE;

        blocksDefaults.atlasRT = LoadRenderTexture(blocksDefaults.atlasW, blocksDefaults.atlasH);
        blocksDefaults.atlasTex = blocksDefaults.atlasRT.texture;
        blocksDefaults.atlasReady = true;

        for (uint16_t id : blocksDefaults.sortedIds) {
            BlockReference& b = blocksDefaults.loaded.blocks[id];
            for (int fi : faceOrder) {
                b.faces[fi].uv = normalizedUVFromIndex(0, 1, 1, blocksDefaults.atlasW, blocksDefaults.atlasH);
            }
        }

        saveAtlasPNG(blocksDefaults, atlasPngOut);
        writeAtlasUVJson(blocksDefaults, atlasUvOut);

        std::cout << "Atlas built: unique textures=0 (dummy 1 tile)\n";
        return;
    }

    int atlasCols = ceilSqrtInt(N);
    int atlasRows = (N + atlasCols - 1) / atlasCols;

    int atlasW = atlasCols * TILE_SIZE;
    int atlasH = atlasRows * TILE_SIZE;

    blocksDefaults.atlasCols = (uint16_t)atlasCols;
    blocksDefaults.atlasRows = (uint16_t)atlasRows;
    blocksDefaults.atlasW = atlasW;
    blocksDefaults.atlasH = atlasH;

    blocksDefaults.atlasRT = LoadRenderTexture(atlasW, atlasH);
    blocksDefaults.atlasTex = blocksDefaults.atlasRT.texture;
    blocksDefaults.atlasReady = true;

    BeginTextureMode(blocksDefaults.atlasRT);
    ClearBackground({0, 0, 0, 0});

    for (int i = 0; i < N; ++i) {
        const std::string& texNameRaw = orderedAtlasTextureNames[i];

        // ✅ CORRECTIF (idem) lors du draw: gestion extension éventuelle
        fs::path p = texturesDir / texNameRaw;
        if (p.extension().empty()) {
            p += ".png";
        }

        Texture2D tex = IOutils::loadTextureOrFallback(p);
        if (tex.id == 0) {
            EndTextureMode();
            throw std::runtime_error("Failed to load texture: " + p.string());
        }

        blocksDefaults.atlasTextures.push_back(tex);

        int col = i % atlasCols;
        int row = i / atlasCols;

        Rectangle src{ 0.0f, 0.0f, (float)tex.width, (float)tex.height };
        Rectangle dst{ (float)(col * TILE_SIZE), (float)(row * TILE_SIZE), (float)TILE_SIZE, (float)TILE_SIZE };

        DrawTexturePro(tex, src, dst, Vector2{0, 0}, 0.0f, WHITE);
    }

    EndTextureMode();

    for (uint16_t id : blocksDefaults.sortedIds) {
        BlockReference& b = blocksDefaults.loaded.blocks[id];
        for (int fi : faceOrder) {
            const std::string& texNameRaw = b.faces[fi].textureName;
            auto it = nameToAtlasIndex.find(texNameRaw);
            if (it == nameToAtlasIndex.end()) {
                throw std::runtime_error("Missing atlas index for texture: " + texNameRaw);
            }
            b.faces[fi].uv = normalizedUVFromIndex(it->second, atlasCols, atlasRows, atlasW, atlasH);
        }
    }

    saveAtlasPNG(blocksDefaults, atlasPngOut);
    writeAtlasUVJson(blocksDefaults, atlasUvOut);

    std::cout << "Atlas built: unique textures=" << N
              << " atlasCols=" << atlasCols
              << " atlasRows=" << atlasRows
              << " atlasSize=" << atlasW << "x" << atlasH << "\n";
}

static inline bool tryLoadSavedAtlasImpl(
    BlocksDefaults& blocksDefaults,
    const fs::path& atlasPngPath,
    const fs::path& atlasUvPath
) {
    if (!fs::exists(atlasPngPath) || !fs::exists(atlasUvPath)) return false;

    unloadAtlas(blocksDefaults);

    Image img = LoadImage(atlasPngPath.string().c_str());
    if (img.width <= 0 || img.height <= 0) {
        UnloadImage(img);
        return false;
    }

    blocksDefaults.atlasW = img.width;
    blocksDefaults.atlasH = img.height;
    blocksDefaults.atlasTex = LoadTextureFromImage(img);
    UnloadImage(img);

    if (blocksDefaults.atlasTex.id == 0) {
        blocksDefaults.atlasTex = Texture2D{};
        return false;
    }

    blocksDefaults.atlasReady = true;
    blocksDefaults.atlasRT = RenderTexture2D{};

    bool ok = loadAtlasUVJson(blocksDefaults, atlasUvPath);
    if (!ok) {
        if (blocksDefaults.atlasTex.id != 0) UnloadTexture(blocksDefaults.atlasTex);
        blocksDefaults.atlasTex = Texture2D{};
        blocksDefaults.atlasReady = false;
        return false;
    }

    return true;
}

void buildAtlasForBlocks(
    BlocksDefaults& blocksDefaults,
    const fs::path& texturesDir,
    const fs::path& atlasPngOut,
    const fs::path& atlasUvOut
) {
    buildAtlasForBlocksImpl(blocksDefaults, texturesDir, atlasPngOut, atlasUvOut);
}

bool tryLoadSavedAtlas(
    BlocksDefaults& blocksDefaults,
    const fs::path& atlasPngPath,
    const fs::path& atlasUvPath
) {
    return tryLoadSavedAtlasImpl(blocksDefaults, atlasPngPath, atlasUvPath);
}
