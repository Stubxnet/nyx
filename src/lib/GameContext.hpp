#ifndef GAME_CONTEXT_HPP
#define GAME_CONTEXT_HPP

#include "World.hpp"

// main game context class
// contains all the information of the game's components

class GameContext {
private:
    World world;

public:
    World getWorld() const {
        return world;
    }

    void setWorld(const World& newWorld) {
        world = newWorld;
    }

};

#endif // GAME_CONTEXT_HPP
