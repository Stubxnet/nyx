#pragma once

#include <nlohmann/json.hpp>
#include <filesystem>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <iostream>
#include <optional>
#include <variant>

#include "../lib/BlockDefaults.hpp"
#include "../lib/BlockData.hpp"
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

static inline BlockType parseBlockType(const json& j, const char* key, const fs::path& path) {
    if (!j.contains(key)) return BlockType::FULL;
    if (!j.at(key).is_string())
        throw std::runtime_error("Field '" + std::string(key) + "' must be a string in " + path.string());

    const std::string s = j.at(key).get<std::string>();
    if (s == "FULL") return BlockType::FULL;
    if (s == "STAIRS") return BlockType::STAIRS;
    if (s == "SLAB") return BlockType::SLAB;
    if (s == "LEAVES") return BlockType::LEAVES;
    if (s == "ORIENTED") return BlockType::ORIENTED;

    throw std::runtime_error("Invalid block type '" + s + "' in " + path.string());
}

static inline BlockMaterial parseBlockMaterial(const json& j, const char* key, const fs::path& path) {
    if (!j.contains(key)) return BlockMaterial::DIRT;
    if (!j.at(key).is_string())
        throw std::runtime_error("Field '" + std::string(key) + "' must be a string in " + path.string());

    const std::string s = j.at(key).get<std::string>();
    if (s == "DIRT") return BlockMaterial::DIRT;
    if (s == "STONE") return BlockMaterial::STONE;
    if (s == "WOOD") return BlockMaterial::WOOD;
    if (s == "VEGETAL") return BlockMaterial::VEGETAL;

    throw std::runtime_error("Invalid material '" + s + "' in " + path.string());
}

static inline Axis parseAxisStr(const std::string& s, const fs::path& path) {
    if (s == "X") return Axis::X;
    if (s == "Y") return Axis::Y;
    if (s == "Z") return Axis::Z;
    throw std::runtime_error("Invalid axis '" + s + "' in " + path.string());
}

static inline Facing6 parseFacing6Str(const std::string& s, const fs::path& path) {
    if (s == "Down")  return Facing6::Down;
    if (s == "Up")    return Facing6::Up;
    if (s == "North") return Facing6::North;
    if (s == "South") return Facing6::South;
    if (s == "West")  return Facing6::West;
    if (s == "East")  return Facing6::East;
    throw std::runtime_error("Invalid facing6 '" + s + "' in " + path.string());
}

static inline std::optional<BlockExtra> parseExtraByType(const json& j, BlockType type, const fs::path& path) {
    // Formats attendus (optionnels) :
    // - "slab":   { "top": bool, "axis": "X|Y|Z" }
    // - "stairs": { "facing": "...", "upsideDown": bool, "side": bool }
    // - "oriented": { "facing": "..." }

    if (type == BlockType::SLAB) {
        if (!j.contains("slab") || !j.at("slab").is_object()) return std::nullopt;

        const json& sj = j.at("slab");
        SlabData sd;

        if (sj.contains("top")) {
            if (!sj.at("top").is_boolean())
                throw std::runtime_error("Field 'slab.top' must be boolean in " + path.string());
            sd.top = sj.at("top").get<bool>();
        }
        if (sj.contains("axis")) {
            if (!sj.at("axis").is_string())
                throw std::runtime_error("Field 'slab.axis' must be string in " + path.string());
            sd.axis = parseAxisStr(sj.at("axis").get<std::string>(), path);
        }

        return BlockExtra{sd};
    }

    if (type == BlockType::STAIRS) {
        if (!j.contains("stairs") || !j.at("stairs").is_object()) return std::nullopt;

        const json& stj = j.at("stairs");
        StairsData sd;

        if (stj.contains("facing")) {
            if (!stj.at("facing").is_string())
                throw std::runtime_error("Field 'stairs.facing' must be string in " + path.string());
            sd.facing = parseFacing6Str(stj.at("facing").get<std::string>(), path);
        }
        if (stj.contains("upsideDown")) {
            if (!stj.at("upsideDown").is_boolean())
                throw std::runtime_error("Field 'stairs.upsideDown' must be boolean in " + path.string());
            sd.upsideDown = stj.at("upsideDown").get<bool>();
        }
        if (stj.contains("side")) {
            if (!stj.at("side").is_boolean())
                throw std::runtime_error("Field 'stairs.side' must be boolean in " + path.string());
            sd.side = stj.at("side").get<bool>();
        }

        return BlockExtra{sd};
    }

    if (type == BlockType::ORIENTED) {
        if (!j.contains("oriented") || !j.at("oriented").is_object()) return std::nullopt;

        const json& oj = j.at("oriented");
        OrientedData od;

        if (oj.contains("facing")) {
            if (!oj.at("facing").is_string())
                throw std::runtime_error("Field 'oriented.facing' must be string in " + path.string());
            od.facing = parseFacing6Str(oj.at("facing").get<std::string>(), path);
        }

        return BlockExtra{od};
    }

    // Pour FULL/LEAVES/etc : on ignore les champs extra.
    return std::nullopt;
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
    b.type = parseBlockType(j, "type", path);
    b.material = parseBlockMaterial(j, "material", path);

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

    // --- AJOUT : extra optionnel via variant ---
    b.extra = parseExtraByType(j, b.type, path);

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
    b.resistance = 16383; // Unbreakable block.
    b.type = parseBlockType(j, "type", path);
    b.material = parseBlockMaterial(j, "material", path);

    for (auto& face : b.faces) {
        face.textureName = tex;
    }

    // --- AJOUT : extra optionnel sur default.json aussi ---
    b.extra = parseExtraByType(j, b.type, path);

    return b;
}

static inline LoadedBlockDefaults loadBlockReferences(const fs::path& dir, const fs::path& textureDir) {
    LoadedBlockDefaults out;

    if (!fs::exists(dir) || !fs::is_directory(dir)) {
        throw std::runtime_error("Invalid blocks directory: " + dir.string());
    }

    const fs::path defaultPath = dir / "default.json";
    if (fs::exists(defaultPath) && fs::is_regular_file(defaultPath)) {
        try {
            out.defaultBlock = parseDefaultBlockJson(defaultPath, textureDir);
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] " << e.what() << '\n';
        }
    } else {
        std::cerr << "[WARNING] default.json not found, missing block IDs will have no fallback texture\n";
    }

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file() || entry.path().extension() != ".json") continue;
        if (entry.path().filename() == "default.json") continue;

        try {
            BlockReference b = parseBlockJson(entry.path(), textureDir);

            auto [it, inserted] = out.blocks.insert_or_assign(b.id, std::move(b));
            if (!inserted) {
                std::cerr << "[WARNING] overwrite block id=" << it->first << '\n';
            }
        } catch (const std::exception& e) {
            std::cerr << "[ERROR] " << e.what() << '\n';
        }
    }

    return out;
}
