#ifndef GAME_CONTEXT_HPP
#define GAME_CONTEXT_HPP

#include "World.hpp"
#include "Player.hpp"
#include "GameRules.hpp"

// main game context class
// contains all the information of the game's components

class GameContext {
private:
    World world;
    Player player1;
    GameRules gamerules;

public:
    World getWorld() const {
        return world;
    }

    // actually multiplayer is not implemented, so one player will be enough
    Player getPlayer1() const {
        return player1;
    }

    GameRules getGameRules() const {
        return gamerules;
    }

    void setWorld(const World& newWorld) {
        world = newWorld;
    }

    void setPlayer1(const Player& newPlayer) {
        player1 = newPlayer;
    }

    void setGameRules(const GameRules& newGameRules) {
        gamerules = newGameRules;
    }
};

#endif // GAME_CONTEXT_HPP
