#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <iostream>
#include <optional>

#include "../lib/BlockDefaults.hpp"
#include "../utils/IO_Utils.cpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

static inline std::string readTrimmedText(const fs::path& path) {
    std::string s = IOutils::readTextFile(path);
    auto first = s.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) return {};
    auto last = s.find_last_not_of(" \t\r\n");
    return s.substr(first, last - first + 1);
}

static inline std::string requireString(const json& j, const char* key, const fs::path& path) {
    if (!j.contains(key))
        throw std::runtime_error("Missing field '" + std::string(key) + "' in " + path.string());
    if (!j.at(key).is_string())
        throw std::runtime_error("Field '" + std::string(key) + "' must be a string in " + path.string());
    return j.at(key).get<std::string>();
}

static inline uint16_t requireU16(const json& j, const char* key, const fs::path& path) {
    if (!j.contains(key))
        throw std::runtime_error("Missing field '" + std::string(key) + "' in " + path.string());
    if (!j.at(key).is_number_integer())
        throw std::runtime_error("Field '" + std::string(key) + "' must be an integer in " + path.string());

    long long v = j.at(key).get<long long>();
    if (v < 0 || v > 65535)
        throw std::runtime_error("Field '" + std::string(key) + "' out of range [0..65535] in " + path.string());

    return static_cast<uint16_t>(v);
}

static inline void validateTextureExists(const fs::path& textureDir, const std::string& textureName, const fs::path& jsonPath) {
    fs::path texturePath = textureDir / textureName;

    if (texturePath.extension().empty()) {
        texturePath += ".png";
    }

    if (!fs::exists(texturePath) || !fs::is_regular_file(texturePath)) {
        throw std::runtime_error("Missing texture '" + textureName + "' referenced by " +
                                 jsonPath.string() + " (" + texturePath.string() + ")");
    }
}

static inline void normalizeTextureNames(BlockReference& b, const fs::path& textureDir, const fs::path& jsonPath) {
    for (auto& face : b.faces) {
        if (face.textureName.empty()) continue;
        if (fs::path(face.textureName).extension().empty()) {
            face.textureName += ".png";
        }
        validateTextureExists(textureDir, face.textureName, jsonPath);
    }
}

static inline BlockReference parseBlockJson(const fs::path& path, const fs::path& textureDir) {
    const std::string raw = readTrimmedText(path);
    if (raw.empty()) {
        throw std::runtime_error("Empty JSON file: " + path.string());
    }

    json j;
    try {
        j = json::parse(raw);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Invalid JSON in " + path.string() + ": " + std::string(e.what()));
    }

    if (!j.is_object()) {
        throw std::runtime_error("Root JSON value must be an object in " + path.string());
    }

    BlockReference b;
    b.id = requireU16(j, "id", path);
    b.displayName = requireString(j, "displayName", path);
    b.resistance = requireU16(j, "resistance", path);

    if (!j.contains("faces") || !j.at("faces").is_object()) {
        throw std::runtime_error("Missing or invalid 'faces' object in " + path.string());
    }

    const auto& faces = j.at("faces");

    b.faces[TOP].textureName    = requireString(faces, "top", path);
    b.faces[BOTTOM].textureName = requireString(faces, "bottom", path);
    b.faces[EAST].textureName   = requireString(faces, "east", path);
    b.faces[WEST].textureName   = requireString(faces, "west", path);
    b.faces[NORTH].textureName  = requireString(faces, "north", path);
    b.faces[SOUTH].textureName  = requireString(faces, "south", path);

    normalizeTextureNames(b, textureDir, path);
    return b;
}

static inline std::optional<BlockReference> parseDefaultBlockJson(const fs::path& path, const fs::path& textureDir) {
    const std::string raw = readTrimmedText(path);
    if (raw.empty()) {
        throw std::runtime_error("Empty JSON file: " + path.string());
    }

    json j;
    try {
        j = json::parse(raw);
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Invalid JSON in " + path.string() + ": " + std::string(e.what()));
    }

    if (!j.is_object()) {
        throw std::runtime_error("Root JSON value must be an object in " + path.string());
    }

    if (!j.contains("texture") || !j.at("texture").is_string()) {
        throw std::runtime_error("Missing field 'texture' in default block file " + path.string());
    }

    std::string tex = j.at("texture").get<std::string>();
    if (fs::path(tex).extension().empty()) {
        tex += ".png";
    }

    validateTextureExists(textureDir, tex, path);

    BlockReference b;
    b.id = 0;
    b.displayName = "default";
    b.resistance = 0;

    for (auto& face : b.faces) {
        face.textureName = tex;
    }

    return b;
}

static inline LoadedBlockDefaults
loadBlockReferences(const fs::path& dir, const fs::path& textureDir) {
    LoadedBlockDefaults out;

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        throw std::runtime_error("Invalid blocks directory: " + dir.string());
    }

    const fs::path defaultPath = dir / "default.json";
    if (fs::exists(defaultPath) && fs::is_regular_file(defaultPath)) {
        out.defaultBlock = parseDefaultBlockJson(defaultPath, textureDir);
    } else {
        std::cerr << "[WARNING] default.json not found, missing block IDs will have no fallback texture\n";
    }

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;
        if (entry.path().filename() == "default.json") continue;

        try {
            BlockReference b = parseBlockJson(entry.path(), textureDir);

            auto [it, inserted] = out.blocks.emplace(b.id, std::move(b));
            if (!inserted) {
                std::cerr << "[WARNING] overwrite block id=" << it->first << '\n';
                it->second = std::move(b);
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] " << e.what() << '\n';
        }
    }

    return out;
}

static inline const BlockReference* GetBlockRef(
    const LoadedBlockDefaults& defaults,
    uint16_t id
) {
    auto it = defaults.blocks.find(id);
    if (it != defaults.blocks.end()) {
        return &it->second;
    }
    if (defaults.defaultBlock.has_value()) {
        return &(*defaults.defaultBlock);
    }
    return nullptr;
}
