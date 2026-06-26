#pragma once

#include <cstdint>

struct RaycastHit
{
    bool hit = false;
    int64_t x = 0, y = 0, z = 0;
    int id = 0;
    float dist = 0.0f;
    int normalX = 0, normalY = 0, normalZ = 0;
};
