#ifndef GAME_HPP
#define GAME_HPP

#include "raylib.h"
#include "rcamera.h"

#include "utils/lib/config.hpp"
#include "utils/drawingUtils.cpp"
#include "utils/colors.cpp"
#include "utils/ioutils.cpp"
#include "generators/void_generator.cpp"

#include "lib/Block.hpp"
#include "lib/Chunk.hpp"
#include "lib/Dimension.hpp"
#include "lib/World.hpp"
#include "lib/GameRules.hpp"

int chunk_size = 16;

class Game {
public:
    void init();
    World initGameData(World& world);
    void run(const Config& config);
};

#endif // GAME_HPP