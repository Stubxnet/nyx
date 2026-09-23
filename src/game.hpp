#ifndef GAME_HPP
#define GAME_HPP

#include "raylib.h"
#include "rcamera.h"

#include <cmath>

#include "utils/colors.cpp"
#include "utils/IO_Utils.cpp"

#include "lib/Block.hpp"
#include "lib/Chunk.hpp"
#include "lib/World.hpp"
#include "lib/Config.hpp"
#include "lib/BlockDefaults.hpp"

#include "render/Mesher.cpp"

#include "tick/TickUpdate.cpp"

#include "ui/Interface.cpp"
#include "ui/drawingUtils.hpp"

#include "data/commands.cpp"
#include "data/AtlasGenerator.cpp"
#include "data/BlocksDefaults.cpp"

#include "core/constants.hpp"
#include "core/enum.hpp"
#include "core/gamestate.hpp"
#include "core/gameinit.cpp"
#include "core/gameupdate.cpp"
#include "core/gamerender.cpp"
#include "core/gamecleanup.cpp"

#endif // GAME_HPP