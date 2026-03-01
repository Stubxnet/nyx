#ifndef GAME_HPP
#define GAME_HPP

#include "raylib.h"

#include "utils/lib/config.hpp"
#include "utils/drawingUtils.cpp"
#include "utils/colors.cpp"
#include "utils/ioutils.cpp"

class Game {
public:
    void init();
    void run(const Config& config);
};

#endif // GAME_HPP