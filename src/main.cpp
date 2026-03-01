#include <raylib.h>
//#include <tinyxml2.h>
#include <SimpleIni.h>
//#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include "utils/utils.cpp"
#include "game.cpp"

using namespace std;

auto main(int [[maybe_unused]] argc, [[maybe_unused]] char *argv[]) -> int {

    if (!std::filesystem::exists("nyx.ini")) {
        std::cerr << "File nyx.ini is required to start game." << std::endl;
        return 1;
    }

    // loading main config
    Config config = readConfig("nyx.ini");

    SetTraceLogLevel(LOG_INFO);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    std::ios_base::sync_with_stdio(false);

    Game game;
    game.run(config);

    return 0;
}