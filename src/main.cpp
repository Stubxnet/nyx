#include <raylib.h>
//#include <tinyxml2.h>
#include <SimpleIni.h>
//#include <nlohmann/json.hpp>
#include <iostream>
#include <filesystem>
#include <string>
#include "utils/utils.cpp"
#include "game.cpp"
#include "lib/Config.hpp"

namespace fs = std::filesystem;

static bool parseCommandLine(int argc, char* argv[],
                             Config &config,
                             bool &disableConfigFile,
                             bool &disableConfigFileAutoInit,
                             std::string &configPath,
                             bool &showHelp,
                             bool &enableVsync,
                             bool &enableMSAA4x,
                             int &overrideTargetFPS,
                             int &overrideRaylibLogLevel) {
        showHelp = false;
            for (int i = 1; i < argc; ++i) {
                std::string a = argv[i];
                if ((a == "--game-dir" || a == "-g") && i + 1 < argc) {
                    config.gameDirectory = argv[++i];
                } else if ((a == "--username" || a == "-u") && i + 1 < argc) {
                    config.username = argv[++i];
                } else if ((a == "--render-distance" || a == "-r") && i + 1 < argc) {
            try {
                config.renderDistance = std::stoi(argv[++i]);
                if (config.renderDistance <= 0) {
                    std::cerr << "Invalid value for --render-distance: must be > 0\n";
                    return false;
                }
            } catch (...) {
                std::cerr << "Invalid value for --render-distance\n";
                return false;
            }
        } else if ((a == "--config-path" || a == "-c") && i + 1 < argc) {
            configPath = argv[++i];
        } else if (a == "--disable-config-file") {
            disableConfigFile = true;
        } else if (a == "--disable-config-file-autoinit") {
            disableConfigFileAutoInit = true;
        } else if (a == "--help" || a == "-h") {
            showHelp = true;
            return true;
        } else if (a == "-v" || a == "--vsync" || a == "--enable-vsync") {
            enableVsync = true;
        } else if (a == "-m" || a == "--msaa" || a == "--enable-msaa-4x") {
            enableMSAA4x = true;
        } else if ((a == "-t" || a == "--target-fps") && i + 1 < argc) {
            try {
                overrideTargetFPS = std::stoi(argv[++i]);
                if (overrideTargetFPS <= 0) {
                    std::cerr << "Invalid value for --target-fps: must be > 0\n";
                    return false;
                }
            } catch (...) {
                std::cerr << "Invalid value for --target-fps\n";
                return false;
            }
        } else if ((a == "-rl" || a == "--raylib-log-level") && i + 1 < argc) {
            std::string lvl = argv[++i];
            if (lvl == "DEBUG") overrideRaylibLogLevel = LOG_DEBUG;
            else if (lvl == "INFO") overrideRaylibLogLevel = LOG_INFO;
            else if (lvl == "WARNING") overrideRaylibLogLevel = LOG_WARNING;
            else if (lvl == "ERROR") overrideRaylibLogLevel = LOG_ERROR;
            else {
                std::cerr << "Invalid value for --raylib-log-level: must be DEBUG | INFO | WARNING | ERROR\n";
                return false;
            }
        } else {
            std::cerr << "Unknown argument: " << a << '\n';
            return false;
        }
    }
    return true;
}

static void printUsage(const char* progName) {
    std::cout << "Usage: " << progName << " [options]\n"
              << "Options:\n"
              << "  -g, --game-dir PATH          Path to game directory (required)\n"
              << "  -u, --username NAME          Username to use\n"
              << "  -r, --render-distance INT  Render distance (positive int)\n"
              << "  -c, --config-path PATH       Path to config file (default: nyx.ini)\n"
              << "      --disable-config-file    Do not read config file\n"
              << "      --disable-config-file-autoinit  Do not auto-create config file\n"
              << "  -v, --vsync, --enable-vsync  Enable VSync (sets FLAG_VSYNC_HINT)\n"
              << "  -m, --msaa, --enable-msaa-4x Enable 4x MSAA (sets FLAG_MSAA_4X_HINT)\n"
              << "  -t, --target-fps INT         Target FPS (overrides config)\n"
              << "  -rl, --raylib-log-level LVL  Raylib log level: DEBUG | INFO | WARNING | ERROR\n"
              << "  -h, --help                   Show this help\n";
}

int main(int argc, char *argv[]) {
    std::string cfgFile = "nyx.ini";
    bool disableConfigFile = false;
    bool disableConfigFileAutoInit = false;
    bool showHelp = false;

    Config config;
    std::string configPathOverride;

    bool enableVsync = false;
    bool enableMSAA4x = false;
    int overrideTargetFPS = -1;
    int overrideRaylibLogLevel = LOG_INFO;

    if (!parseCommandLine(argc, argv, config, disableConfigFile, disableConfigFileAutoInit, configPathOverride, showHelp, enableVsync, enableMSAA4x, overrideTargetFPS, overrideRaylibLogLevel)) {
        printUsage(argv[0]);
        return 2;
    }

    if (showHelp) {
        printUsage(argv[0]);
        return 0;
    }

    if (!configPathOverride.empty()) {
        cfgFile = configPathOverride;
    }

    if (!fs::exists(cfgFile) && !disableConfigFileAutoInit) {
        if (!initDefaultConfigFile(cfgFile)) {
            std::cerr << "Warning: failed to create default config file '" << cfgFile << "'\n";
        }
    }

    if (!disableConfigFile && fs::exists(cfgFile)) {
        try {
            config = readConfig(cfgFile);
        } catch (const std::exception &e) {
            std::cerr << "Warning: failed to load config file '" << cfgFile << "': " << e.what() << " — using defaults\n";
        }
    } else {
        config.windowWidth = 800;
        config.windowHeight = 600;
        config.targetFPS = 60;
        config.windowTitle = "Nyx";
        config.gamma = 2.2f;
        if (config.username.empty()) config.username = "DefaultUser";
        if (config.renderDistance <= 0) config.renderDistance = 12;
    }

    bool dummyDisable1 = disableConfigFile;
    bool dummyDisable2 = disableConfigFileAutoInit;
    std::string dummyPath;
    bool dummyHelp = false;

    if (!parseCommandLine(argc, argv, config, dummyDisable1, dummyDisable2, dummyPath, dummyHelp, enableVsync, enableMSAA4x, overrideTargetFPS, overrideRaylibLogLevel)) {
        std::cerr << "Error parsing command line arguments\n";
        printUsage(argv[0]);
        return 2;
    }

    if (overrideTargetFPS > 0) {
        config.targetFPS = overrideTargetFPS;
    }

    if (config.gameDirectory.empty()) {
        std::cerr << "Error: --game-dir is required.\n";
        printUsage(argv[0]);
        return 1;
    }

    if (config.windowWidth <= 0 || config.windowHeight <= 0) {
        std::cerr << "Invalid window dimensions; must be positive. Using defaults.\n";
        config.windowWidth = 800;
        config.windowHeight = 600;
    }
    if (config.targetFPS <= 0) {
        std::cerr << "Invalid target FPS; must be positive. Using default 60.\n";
        config.targetFPS = 60;
    }

    SetTraceLogLevel(overrideRaylibLogLevel);

    if (enableVsync) SetConfigFlags(FLAG_VSYNC_HINT);
    if (enableMSAA4x) SetConfigFlags(FLAG_MSAA_4X_HINT);

    std::ios_base::sync_with_stdio(false);

    try {
        run(config);
    } catch (const std::exception &e) {
        std::cerr << "Fatal error while running: " << e.what() << '\n';
        return 3;
    } catch (...) {
        std::cerr << "Unknown fatal error while running\n";
        return 3;
    }

    return 0;
}
