#ifndef BLOCK_H
#define BLOCK_H

#include <string>
#include "raylib.h"

class Block {
private:
    Vector3 position;
    int32_t id;
    std::string texture;
    int32_t modelid;
    bool breakable;
    int32_t resistance;
    bool notexture;
    int32_t hitbox;
    std::string miningsound;
    std::string breaksound;
    std::string walksound;
    bool natural;

public:
    Block(float x = 0.0f, float y = 0.0f, float z = 0.0f, int32_t id = 0) 
        : position({x, y, z}), id(id), breakable(false), notexture(false), natural(false) {}

    ~Block() {}

    Vector3 getPosition() const { return position; }
    void setPosition(float x, float y, float z) { position = {x, y, z}; }

    void setId(int32_t id) { this->id = id; }
    int32_t getId() const { return id; }

    void setTexture(const std::string& tex) { texture = tex; }
    const std::string& getTexture() const { return texture; }

    void setModelId(int32_t modelId) { modelid = modelId; }
    int32_t getModelId() const { return modelid; }

    void setBreakable(bool breakable) { this->breakable = breakable; }
    bool isBreakable() const { return breakable; }

    void setResistance(int32_t resistance) { this->resistance = resistance; }
    int32_t getResistance() const { return resistance; }

    void setNoTexture(bool noTexture) { notexture = noTexture; }
    bool hasNoTexture() const { return notexture; }

    void setHitbox(int32_t hitbox) { this->hitbox = hitbox; }
    int32_t getHitbox() const { return hitbox; }

    void setMiningSound(const std::string& sound) { miningsound = sound; }
    const std::string& getMiningSound() const { return miningsound; }

    void setBreakSound(const std::string& sound) { breaksound = sound; }
    const std::string& getBreakSound() const { return breaksound; }

    void setWalkSound(const std::string& sound) { walksound = sound; }
    const std::string& getWalkSound() const { return walksound; }

    void setNatural(bool natural) { this->natural = natural; }
    bool isNatural() const { return natural; }
};

#endif
