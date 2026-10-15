#pragma once

#include <nlohmann/json.hpp>

#include <cstdint>
#include <filesystem>
#include <iostream>
#include <limits>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "../data/lib/BlocksDefaults.hpp"
#include "lib/BlockData.hpp"
#include "../utils/utils.cpp"

namespace fs = std::filesystem;
using json = nlohmann::json;

static inline std::string readTrimmedText(const fs::path& path) {
    std::string text = readTextFile(path);

    const auto first = text.find_first_not_of(" \t\r\n");
    if (first == std::string::npos) { return {}; }

    const auto last = text.find_last_not_of(" \t\r\n");
    return text.substr(first, last - first + 1);
}

namespace block_json_detail {

static inline std::string location(const fs::path& path) { return path.string(); }

static inline const json& requireField(const json& object, const char* key, const fs::path& path) {
    if (!object.contains(key)) { throw std::runtime_error("Missing field '" + std::string(key) + "' in " + location(path)); }

    return object.at(key);
}

static inline const json* optionalField(const json& object, const char* key) {
    if (!object.contains(key)) { return nullptr; }

    return &object.at(key);
}

static inline const json& requireObject(const json& object, const char* key, const fs::path& path) {
    const json& value = requireField(object, key, path);

    if (!value.is_object()) { throw std::runtime_error("Field '" + std::string(key) + "' must be an object in " + location(path)); }

    return value;
}

static inline std::string requireString(const json& object, const char* key, const fs::path& path) {
    const json& value = requireField(object, key, path);

    if (!value.is_string()) { throw std::runtime_error("Field '" + std::string(key) + "' must be a string in " + location(path)); }

    return value.get<std::string>();
}

static inline std::uint16_t requireU16(const json& object, const char* key, const fs::path& path) {
    const json& value = requireField(object, key, path);

    if (!value.is_number_integer()) { throw std::runtime_error("Field '" + std::string(key) + "' must be an integer in " + location(path)); }

    std::uint64_t number = 0;

    if (value.is_number_unsigned()) {
        number = value.get<std::uint64_t>();
    } else {
        const std::int64_t signedNumber = value.get<std::int64_t>();

        if (signedNumber < 0) { throw std::runtime_error("Field '" + std::string(key) + "' out of range [0..65535] in " + location(path)); }
        number = static_cast<std::uint64_t>(signedNumber);
    }

    if (number > std::numeric_limits<std::uint16_t>::max()) { throw std::runtime_error("Field '" + std::string(key) + "' out of range [0..65535] in " + location(path)); }

    return static_cast<std::uint16_t>(number);
}

static inline std::string requireStringValue(const json& value, const std::string& fieldName, const fs::path& path) {
    if (!value.is_string()) { throw std::runtime_error("Field '" + fieldName + "' must be a string in " + location(path)); }

    return value.get<std::string>();
}

static inline bool requireBoolValue(const json& value, const std::string& fieldName, const fs::path& path) {
    if (!value.is_boolean()) { throw std::runtime_error("Field '" + fieldName + "' must be boolean in " + location(path)); }

    return value.get<bool>();
}


template <typename Enum> static inline Enum parseEnum(const std::string& value, const std::unordered_map<std::string, Enum>& values, const std::string& fieldName, const fs::path& path) {
    const auto it = values.find(value);

    if (it == values.end()) { throw std::runtime_error("Invalid " + fieldName + " '" + value + "' in " + location(path)); }

    return it->second;
}

static inline const std::unordered_map<std::string, BlockType>& blockTypes() {
    static const std::unordered_map<std::string, BlockType> values{
        {"FULL", BlockType::FULL},
        {"STAIRS", BlockType::STAIRS},
        {"SLAB", BlockType::SLAB},
        {"LEAVES", BlockType::LEAVES},
        {"ORIENTED", BlockType::ORIENTED},
    };
    return values;
}

static inline const std::unordered_map<std::string, BlockMaterial>& materials() {
    static const std::unordered_map<std::string, BlockMaterial> values{
        {"DIRT", BlockMaterial::DIRT},
        {"SAND", BlockMaterial::SAND},
        {"STONE", BlockMaterial::STONE},
        {"WOOD", BlockMaterial::WOOD},
        {"VEGETAL", BlockMaterial::VEGETAL},
        {"LIQUID", BlockMaterial::LIQUID},
    };
    return values;
}

static inline const std::unordered_map<std::string, Axis>& axes() {
    static const std::unordered_map<std::string, Axis> values{
        {"X", Axis::X},
        {"Y", Axis::Y},
        {"Z", Axis::Z},
    };
    return values;
}

static inline const std::unordered_map<std::string, Facing6>& facings() {
    static const std::unordered_map<std::string, Facing6> values{
        {"Down", Facing6::Down}, {"Up", Facing6::Up}, {"North", Facing6::North}, {"South", Facing6::South}, {"West", Facing6::West}, {"East", Facing6::East},
    };
    return values;
}

template <typename Enum>
static inline Enum parseOptionalEnumField(const json& object, const char* key, const std::string& fieldName, const std::unordered_map<std::string, Enum>& values, Enum defaultValue,
                                          const fs::path& path) {
    const json* value = optionalField(object, key);
    if (value == nullptr) { return defaultValue; }

    const std::string text = requireStringValue(*value, fieldName, path);
    return parseEnum(text, values, fieldName, path);
}

static inline bool parseOptionalBoolField(const json& object, const char* key, const std::string& fieldName, bool defaultValue, const fs::path& path) {
    const json* value = optionalField(object, key);
    if (value == nullptr) { return defaultValue; }

    return requireBoolValue(*value, fieldName, path);
}

static inline std::optional<SlabData> parseSlabData(const json& object, const fs::path& path) {
    const json* slabJson = optionalField(object, "slab");

    if (slabJson == nullptr) { return std::nullopt; }

    if (!slabJson->is_object()) { return std::nullopt; }

    SlabData data;

    data.top = parseOptionalBoolField(*slabJson, "top", "slab.top", data.top, path);

    data.axis = parseOptionalEnumField(*slabJson, "axis", "slab.axis", axes(), data.axis, path);

    return data;
}

static inline std::optional<StairsData> parseStairsData(const json& object, const fs::path& path) {
    const json* stairsJson = optionalField(object, "stairs");

    if (stairsJson == nullptr) { return std::nullopt; }

    if (!stairsJson->is_object()) { return std::nullopt; }

    StairsData data;

    data.facing = parseOptionalEnumField(*stairsJson, "facing", "stairs.facing", facings(), data.facing, path);

    data.upsideDown = parseOptionalBoolField(*stairsJson, "upsideDown", "stairs.upsideDown", data.upsideDown, path);

    data.side = parseOptionalBoolField(*stairsJson, "side", "stairs.side", data.side, path);

    return data;
}

static inline std::optional<OrientedData> parseOrientedData(const json& object, const fs::path& path) {
    const json* orientedJson = optionalField(object, "oriented");

    if (orientedJson == nullptr) { return std::nullopt; }

    if (!orientedJson->is_object()) { return std::nullopt; }

    OrientedData data;

    data.facing = parseOptionalEnumField(*orientedJson, "facing", "oriented.facing", facings(), data.facing, path);

    return data;
}

static inline json readJsonObject(const fs::path& path) {
    const std::string raw = readTrimmedText(path);

    if (raw.empty()) { throw std::runtime_error("Empty JSON file: " + path.string()); }

    json parsed;

    try {
        parsed = json::parse(raw);
    } catch (const json::parse_error& error) { throw std::runtime_error("Invalid JSON in " + path.string() + ": " + error.what()); }

    if (!parsed.is_object()) { throw std::runtime_error("Root JSON value must be an object in " + path.string()); }

    return parsed;
}

} // namespace block_json_detail



static inline std::string requireString(const json& j, const char* key, const fs::path& path) { return block_json_detail::requireString(j, key, path); }

static inline std::uint16_t requireU16(const json& j, const char* key, const fs::path& path) { return block_json_detail::requireU16(j, key, path); }

static inline void validateTextureExists(const fs::path& textureDir, const std::string& textureName, const fs::path& jsonPath) {
    const fs::path texturePath = textureDir / textureName;

    if (!fs::exists(texturePath) || !fs::is_regular_file(texturePath)) {
        throw std::runtime_error("Missing texture '" + textureName + "' referenced by " + jsonPath.string() + " (" + texturePath.string() + ")");
    }
}

static inline void normalizeTextureNames(BlockReference& block, const fs::path& textureDir, const fs::path& jsonPath) {
    for (auto& face : block.faces) {
        if (face.textureName.empty()) { continue; }

        if (fs::path(face.textureName).extension().empty()) { face.textureName += ".png"; }

        validateTextureExists(textureDir, face.textureName, jsonPath);
    }
}

static inline BlockType parseBlockType(const json& j, const char* key, const fs::path& path) {
    const json* value = block_json_detail::optionalField(j, key);
    if (value == nullptr) { return BlockType::FULL; }

    const std::string name = block_json_detail::requireStringValue(*value, key, path);

    return block_json_detail::parseEnum(name, block_json_detail::blockTypes(), "block type", path);
}

static inline BlockMaterial parseBlockMaterial(const json& j, const char* key, const fs::path& path) {
    const json* value = block_json_detail::optionalField(j, key);
    if (value == nullptr) { return BlockMaterial::SAND; }

    const std::string name = block_json_detail::requireStringValue(*value, key, path);

    return block_json_detail::parseEnum(name, block_json_detail::materials(), "material", path);
}

static inline Axis parseAxisStr(const std::string& value, const fs::path& path) { return block_json_detail::parseEnum(value, block_json_detail::axes(), "axis", path); }

static inline Facing6 parseFacing6Str(const std::string& value, const fs::path& path) { return block_json_detail::parseEnum(value, block_json_detail::facings(), "facing6", path); }

static inline std::optional<BlockExtra> parseExtraByType(const json& j, BlockType type, const fs::path& path) {
    switch (type) {
    case BlockType::SLAB: {
        const auto data = block_json_detail::parseSlabData(j, path);
        if (data) return BlockExtra{*data};
        return std::nullopt;
    }

    case BlockType::STAIRS: {
        const auto data = block_json_detail::parseStairsData(j, path);
        if (data) return BlockExtra{*data};
        return std::nullopt;
    }

    case BlockType::ORIENTED: {
        const auto data = block_json_detail::parseOrientedData(j, path);
        if (data) return BlockExtra{*data};
        return std::nullopt;
    }

    case BlockType::FULL:
    case BlockType::LEAVES:
    default:
        return std::nullopt;
    }
}

static inline BlockReference parseBlockJson(const fs::path& path, const fs::path& textureDir) {
    const json j = block_json_detail::readJsonObject(path);

    BlockReference block;
    block.id = requireU16(j, "id", path);
    block.displayName = requireString(j, "displayName", path);
    block.name = requireString(j, "name", path);
    block.resistance = requireU16(j, "resistance", path);
    block.type = parseBlockType(j, "type", path);
    block.material = parseBlockMaterial(j, "material", path);

    const json& faces = block_json_detail::requireObject(j, "faces", path);

    block.faces[TOP].textureName = block_json_detail::requireString(faces, "top", path);
    block.faces[BOTTOM].textureName = block_json_detail::requireString(faces, "bottom", path);
    block.faces[EAST].textureName = block_json_detail::requireString(faces, "east", path);
    block.faces[WEST].textureName = block_json_detail::requireString(faces, "west", path);
    block.faces[NORTH].textureName = block_json_detail::requireString(faces, "north", path);
    block.faces[SOUTH].textureName = block_json_detail::requireString(faces, "south", path);

    block.extra = parseExtraByType(j, block.type, path);

    normalizeTextureNames(block, textureDir, path);
    return block;
}

static inline std::optional<BlockReference> parseDefaultBlockJson(const fs::path& path, const fs::path& textureDir) {
    const json j = block_json_detail::readJsonObject(path);

    std::string texture = requireString(j, "texture", path);

    if (fs::path(texture).extension().empty()) { texture += ".png"; }

    validateTextureExists(textureDir, texture, path);

    BlockReference block;
    block.id = 0;
    block.displayName = "default";
    block.resistance = 16383; // Unbreakable block.
    block.type = parseBlockType(j, "type", path);
    block.material = parseBlockMaterial(j, "material", path);

    for (auto& face : block.faces) { face.textureName = texture; }

    block.extra = parseExtraByType(j, block.type, path);
    return block;
}

static inline LoadedBlockDefaults loadBlockReferences(const fs::path& dir, const fs::path& textureDir) {
    LoadedBlockDefaults result;

    if (!fs::exists(dir) || !fs::is_directory(dir)) { throw std::runtime_error("Invalid blocks directory: " + dir.string()); }

    const fs::path defaultPath = dir / "default.json";

    if (fs::exists(defaultPath) && fs::is_regular_file(defaultPath)) {
        try {
            result.defaultBlock = parseDefaultBlockJson(defaultPath, textureDir);
        } catch (const std::exception& error) { std::cerr << "[ERROR] " << error.what() << '\n'; }
    } else {
        std::cerr << "[WARNING] default.json not found, missing block IDs "
                  << "will have no fallback texture\n";
    }

    for (const auto& entry : fs::directory_iterator(dir)) {
        if (!entry.is_regular_file()) { continue; }

        if (entry.path().extension() != ".json") { continue; }

        if (entry.path().filename() == "default.json") { continue; }

        try {
            BlockReference block = parseBlockJson(entry.path(), textureDir);

            const auto [it, inserted] = result.blocks.insert_or_assign(block.id, std::move(block));

            if (!inserted) { std::cerr << "[WARNING] overwrite block id=" << it->first << '\n'; }
        } catch (const std::exception& error) { std::cerr << "[ERROR] " << error.what() << '\n'; }
    }

    return result;
}
