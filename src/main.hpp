#ifndef GAME_HPP
#define GAME_HPP

#include "raylib.h"
#include "rcamera.h"

#include <cmath>
#include <nlohmann/json.hpp>
#include <exception>
#include <iostream>
#include <filesystem>
#include <string>
#include <cstdint>
//#include <tinyxml2.h>
#include "main.hpp"

#include "utils/colors.cpp"
#include "utils/utils.cpp"

#include "world/lib/Block.hpp"
#include "world/lib/Chunk.hpp"
#include "world/lib/VerticalChunk.hpp"
#include "world/lib/World.hpp"
#include "world/lib/Biome.hpp"


#include "render/Mesher.cpp"

#include "tick/TickUpdate.cpp"

#include "world/WorldGeneration.cpp"

#include "ui/Interface.cpp"
#include "ui/drawingUtils.hpp"

#include "data/commands.cpp"
#include "data/AtlasGenerator.cpp"
#include "data/BlocksDefaults.cpp"
#include "data/LoadConfig.cpp"
#include "data/lib/Config.hpp"
#include "data/lib/BlocksDefaults.hpp"

#include "core/constants.hpp"
#include "core/enum.hpp"
#include "core/gamestate.hpp"
#include "core/gameinit.cpp"
#include "core/gameupdate.cpp"
#include "core/gamerender.cpp"
#include "core/gamecleanup.cpp"

#endif // GAME_HPP