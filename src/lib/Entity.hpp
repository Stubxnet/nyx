#pragma once
#include "../constants.hpp"
#include "../enum.hpp"
#include "raylib.h"

class Entity {
public:
    int getEid() const { return eid; }
    EntityType getType() const { return type; }
    MoveMode getMode() const { return mode; }
    std::shared_ptr<Body> getBody() const { return body; }
    std::shared_ptr<BoundingBox> getBox() const { return box; }
    Vector3 getPosition() const { return position; }

    void setEid(int id) { eid = id; }
    void setType(EntityType t) { type = t; }
    void setMode(MoveMode m) { mode = m; }
    void setBody(const std::shared_ptr<Body>& b) { body = b; }
    void setBody(std::shared_ptr<Body>&& b) { body = std::move(b); }
    void setBox(const std::shared_ptr<BoundingBox>& bx) { box = bx; }
    void setBox(std::shared_ptr<BoundingBox>&& bx) { box = std::move(bx); }
    void setPosition(Vector3& newPosition) { position = newPosition; }

private:
    int eid{};
    Vector3 position;
    EntityType type{};
    MoveMode mode{};
    std::shared_ptr<Body> body;
    std::shared_ptr<BoundingBox> box;
};
