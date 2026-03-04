#ifndef ENTITY
#define ENTITY

#include "raylib.h"

#include <string>
#include <vector>
#include <stdexcept>

#include "Attributes.hpp"

enum class EntityType {
    PLAYER,
    MOB,
    TECHNICAL
};

class Entity {
private:
    int32_t uuid;
    std::string name;
    Vector3 position;
    Vector3 motion;
    Attributes attributes;
    EntityType type;
    Model model;
    std::vector<std::string> textures;

public:
    Entity(int32_t id, const std::string& entityName, const Vector3& initialPosition, EntityType entityType)
        : uuid(id), name(entityName), position(initialPosition), motion({0}), type(entityType) {
    }

    ~Entity() {
        UnloadModel(model);
    }

    int32_t getUUID() const { return uuid; }
    std::string getName() const { return name; }
    Vector3 getPosition() const { return position; }
    Vector3 getMotion() const { return motion; }
    EntityType getType() const { return type; }
    Attributes getAttributes() const { return attributes; }
    Model getModel() const { return model; }
    const std::vector<std::string>& getTextures() const { return textures; }

    void setName(const std::string& entityName) { name = entityName; }
    void setPosition(const Vector3& newPosition) { position = newPosition; }
    void setMotion(const Vector3& newMotion) { motion = newMotion; }
    void setType(EntityType entityType) { type = entityType; }
    void setAttributes(Attributes newAttributes) { attributes = newAttributes; }
    void setModel(const Model& newModel) { model = newModel; }
    
    void addTexture(const std::string& texturePath) {
        textures.push_back(texturePath);
    }
};

#endif // ENTITY
