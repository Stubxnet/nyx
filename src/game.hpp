#ifndef GAME_HPP
#define GAME_HPP

#include "raylib.h"
#include "rcamera.h"

#include "utils/lib/config.hpp"
#include "utils/drawingUtils.cpp"
#include "utils/colors.cpp"
#include "utils/ioutils.cpp"

#include "lib/GameRules.hpp"
#include "lib/Entity.hpp"
#include "lib/Block.hpp"
#include "lib/Chunk.hpp"
#include "lib/World.hpp"

#include "render/3DrenderingUtils.cpp"

int chunk_size = 16;

class Game {
public:
    void init();
    void run(const Config& config);
};

#endif // GAME_HPP