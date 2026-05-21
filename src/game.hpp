#ifndef GAME_HPP
#define GAME_HPP

#include "raylib.h"
#include "rcamera.h"

#include <cmath>

#include "utils/lib/config.hpp"
#include "utils/colors.cpp"
#include "utils/ioutils.cpp"

#include "lib/GameMode.hpp"
#include "lib/GameRules.hpp"
#include "lib/Entity.hpp"
#include "lib/Block.hpp"
#include "lib/Chunk.hpp"
#include "lib/World.hpp"

#include "render/renderutils.cpp"
#include "render/Mesher.cpp"

#include "textures/atlas.cpp"

#include "ui/Interface.cpp"
#include "ui/drawingUtils.hpp"

#include "constants.hpp"

int chunk_size = 16;

#endif // GAME_HPP