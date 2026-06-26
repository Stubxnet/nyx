#include "raylib.h"
#include "raymath.h"

#include <cmath>
#include <cstdint>

#include "World.hpp"
#include "RaycastHit.hpp"

static inline int Signf(float v)
{
    return (v > 0.0f) - (v < 0.0f);
}

RaycastHit DDA_RaycastWorld(const Ray& ray, const std::shared_ptr<World>& world, float maxDist)
{
    RaycastHit out;
    if (!world) return out;

    Vector3 origin = ray.position;
    Vector3 dir = Vector3Normalize(ray.direction);

    int64_t x = (int64_t)floorf(origin.x);
    int64_t y = (int64_t)floorf(origin.y);
    int64_t z = (int64_t)floorf(origin.z);

    int stepX = Signf(dir.x);
    int stepY = Signf(dir.y);
    int stepZ = Signf(dir.z);

    float invX = (dir.x != 0.0f) ? fabsf(1.0f / dir.x) : 1e30f;
    float invY = (dir.y != 0.0f) ? fabsf(1.0f / dir.y) : 1e30f;
    float invZ = (dir.z != 0.0f) ? fabsf(1.0f / dir.z) : 1e30f;

    float tDeltaX = invX;
    float tDeltaY = invY;
    float tDeltaZ = invZ;

    float tMaxX = (dir.x > 0.0f) ? (((float)x + 1.0f - origin.x) * invX) : ((origin.x - (float)x) * invX);
    float tMaxY = (dir.y > 0.0f) ? (((float)y + 1.0f - origin.y) * invY) : ((origin.y - (float)y) * invY);
    float tMaxZ = (dir.z > 0.0f) ? (((float)z + 1.0f - origin.z) * invZ) : ((origin.z - (float)z) * invZ);

    int lastAxis = -1;
    float t = 0.0f;

    int id0 = world->GetBlockId(x, y, z);
    if (id0 != 0)
    {
        out.hit = true;
        out.x = x; out.y = y; out.z = z;
        out.id = id0;
        out.dist = 0.0f;
        return out;
    }

    while (t <= maxDist)
    {
        if (tMaxX < tMaxY)
        {
            if (tMaxX < tMaxZ)
            {
                x += stepX;
                t = tMaxX;
                tMaxX += tDeltaX;
                lastAxis = 0;
            }
            else
            {
                z += stepZ;
                t = tMaxZ;
                tMaxZ += tDeltaZ;
                lastAxis = 2;
            }
        }
        else
        {
            if (tMaxY < tMaxZ)
            {
                y += stepY;
                t = tMaxY;
                tMaxY += tDeltaY;
                lastAxis = 1;
            }
            else
            {
                z += stepZ;
                t = tMaxZ;
                tMaxZ += tDeltaZ;
                lastAxis = 2;
            }
        }

        int id = world->GetBlockId(x, y, z);
        if (id != 0)
        {
            out.hit = true;
            out.x = x; out.y = y; out.z = z;
            out.id = id;
            out.dist = t;
            out.normalX = out.normalY = out.normalZ = 0;

            if (lastAxis == 0) out.normalX = -stepX;
            if (lastAxis == 1) out.normalY = -stepY;
            if (lastAxis == 2) out.normalZ = -stepZ;

            return out;
        }
    }

    return out;
}
