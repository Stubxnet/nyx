#pragma once
#include <cstdint>


enum class Axis : uint8_t {
    X = 0,
    Y = 1,
    Z = 2
};

enum class Facing6 : uint8_t {
    Down = 0,
    Up   = 1,
    North = 2,
    South = 3,
    West  = 4,
    East  = 5
};

struct SlabData {
    bool top = false;
    Axis axis = Axis::Y;
};

struct StairsData {
    Facing6 facing = Facing6::North;
    bool upsideDown = false;
    bool side = false;
};

struct OrientedData {
    Facing6 facing = Facing6::North;
};

struct BlockExtraPacked {
    uint16_t bits = 0;
};

using BlockExtra = std::variant<std::monostate, SlabData, StairsData, OrientedData>;
