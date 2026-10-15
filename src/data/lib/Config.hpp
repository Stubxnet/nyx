#pragma once

#include <cstdint>
#include <string>
#include "raylib.h"

struct Config {
    int windowWidth = 1500;
    int windowHeight = 800;
    int targetFPS = 60;

    std::string gameDirectory;
    std::string username = "DefaultUser";
    std::string windowTitle = "Nyx";

    int renderDistance = 2;
    float gamma = 2.2f;
    bool atlasRegeneration = false;
    int32_t seed = 1337;

    bool vsync = false;
    bool msaa4x = false;
    int raylibLogLevel = LOG_INFO;
};
