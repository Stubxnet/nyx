#ifndef PLAYER_HPP
#define PLAYER_HPP

#include <string>
#include <glm/vec3.hpp>

class Player {
public:
    Player(float x, float y, float z, const std::string& name, int uuid, 
           const std::string& skin, int fov, int render_distance);

    const glm::vec3& getPosition() const;
    const glm::vec3& getChunk() const;
    const std::string& getName() const;
    int getUUID() const;
    const std::string& getSkin() const;
    int getFOV() const;
    int getRenderDistance() const;

private:
    glm::vec3 position;          // Player position
    glm::vec3 chunk;             // Chunk
    std::string name;            // Player name
    int uuid;                    // UUID
    std::string skin;            // Player skin file path
    int fov;                     // Field of View
    int render_distance;         // Render distance
};

Player::Player(float x, float y, float z, const std::string& name, int uuid, 
               const std::string& skin, int fov, int render_distance)
    : position(x, y, z), chunk(0.0f, 0.0f, 0.0f), name(name), uuid(uuid),
      skin(skin), fov(fov), render_distance(render_distance) {}

const glm::vec3& Player::getPosition() const {
    return position;
}

const glm::vec3& Player::getChunk() const {
    return chunk;
}

const std::string& Player::getName() const {
    return name;
}

int Player::getUUID() const {
    return uuid;
}

const std::string& Player::getSkin() const {
    return skin;in
}

int Player::getFOV() const {
    return fov;
}

int Player::getRenderDistance() const {
    return render_distance;
}

#endif // PLAYER_HPP
