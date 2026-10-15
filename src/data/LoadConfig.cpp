#include "../data/lib/Config.hpp"
#include "../core/enum.hpp"

#include <SimpleIni.h>

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <string>
#include <unordered_set>

namespace fs = std::filesystem;

namespace {

int parseLogLevel(const std::string& value) {
    if (value == "DEBUG")   return 2;
    if (value == "INFO")    return 3;
    if (value == "WARNING") return 4;
    if (value == "ERROR")   return 5;
    throw std::runtime_error(
        "raylib-log-level doit être DEBUG, INFO, WARNING ou ERROR");
}

Config readConfigFile(const std::string& filename) {
    Config config;

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetMultiKey(false);

    if (ini.LoadFile(filename.c_str()) < 0) {
        throw std::runtime_error("impossible de lire le fichier de configuration");
    }

    auto getString = [&](const char* key, const std::string& fallback) {
        const char* value = ini.GetValue("Settings", key, nullptr);
        return value ? std::string(value) : fallback;
    };

    auto getInt = [&](const char* key, int fallback) {
        const char* value = ini.GetValue("Settings", key, nullptr);
        if (!value) return fallback;
        try {
            return std::stoi(value);
        } catch (...) {
            return fallback;
        }
    };

    auto getFloat = [&](const char* key, float fallback) {
        const char* value = ini.GetValue("Settings", key, nullptr);
        if (!value) return fallback;
        try {
            return std::stof(value);
        } catch (...) {
            return fallback;
        }
    };

    auto getBool = [&](const char* key, bool fallback) {
        const char* value = ini.GetValue("Settings", key, nullptr);
        if (!value) return fallback;

        const std::string text(value);
        if (text == "true" || text == "1" || text == "yes") return true;
        if (text == "false" || text == "0" || text == "no") return false;
        return fallback;
    };

    config.gameDirectory = getString("GameDirectory", config.gameDirectory);
    config.username = getString("Username", config.username);
    config.windowTitle = getString("WindowTitle", config.windowTitle);

    config.windowWidth = getInt("WindowWidth", config.windowWidth);
    config.windowHeight = getInt("WindowHeight", config.windowHeight);
    config.targetFPS = getInt("TargetFPS", config.targetFPS);
    config.renderDistance = getInt("RenderDistance", config.renderDistance);
    config.seed = getInt("Seed", config.seed);

    config.gamma = getFloat("Gamma", config.gamma);
    config.atlasRegeneration =
        getBool("AtlasRegeneration", config.atlasRegeneration);
    config.vsync = getBool("VSync", config.vsync);
    config.msaa4x = getBool("MSAA4x", config.msaa4x);

    const std::string logLevel = getString("RaylibLogLevel", "INFO");
    try {
        config.raylibLogLevel = parseLogLevel(logLevel);
    } catch (...) {
        // NOPE
    }

    return config;
}

void printUsage(const char* program) {
    std::cout
        << "Usage: " << program << " [options]\n"
        << "  -g, --game-dir PATH\n"
        << "  -u, --username NAME\n"
        << "  -r, --render-distance INT\n"
        << "      --window-width INT\n"
        << "      --window-height INT\n"
        << "      --window-title TEXT\n"
        << "      --gamma FLOAT\n"
        << "  -s, --seed INT\n"
        << "  -t, --target-fps INT\n"
        << "      --vsync | --no-vsync\n"
        << "      --msaa | --no-msaa\n"
        << "      --regen-atlas | --no-regen-atlas\n"
        << "  -rl, --raylib-log-level DEBUG|INFO|WARNING|ERROR\n"
        << "  -c, --config-path PATH\n"
        << "      --disable-config-file\n"
        << "      --disable-config-file-autoinit\n"
        << "  -h, --help\n";
}

} // namespace

ConfigResult loadConfig(int argc, char* argv[]) {
    Config cli;
    std::unordered_set<std::string> supplied;

    std::string configPath = "nyx.ini";
    bool disableConfigFile = false;
    bool disableConfigFileAutoInit = false;
    ConfigResult result;

    auto readValue = [&](int& i, const std::string& option) -> std::string {
        if (i + 1 >= argc) {
            throw std::runtime_error("valeur manquante pour " + option);
        }
        return argv[++i];
    };

    try {
        for (int i = 1; i < argc; ++i) {
            const std::string arg = argv[i];

            if (arg == "-h" || arg == "--help") {
                result.showHelp = true;
                return result;
            } else if (arg == "-c" || arg == "--config-path") {
                configPath = readValue(i, arg);
            } else if (arg == "--disable-config-file") {
                disableConfigFile = true;
            } else if (arg == "--disable-config-file-autoinit") {
                disableConfigFileAutoInit = true;
            } else if (arg == "-g" || arg == "--game-dir") {
                cli.gameDirectory = readValue(i, arg);
                supplied.insert("GameDirectory");
            } else if (arg == "-u" || arg == "--username") {
                cli.username = readValue(i, arg);
                supplied.insert("Username");
            } else if (arg == "-r" || arg == "--render-distance") {
                cli.renderDistance = std::stoi(readValue(i, arg));
                supplied.insert("RenderDistance");
            } else if (arg == "-s" || arg == "--seed") {
                cli.seed = std::stoi(readValue(i, arg));
                supplied.insert("Seed");
            } else if (arg == "-t" || arg == "--target-fps") {
                cli.targetFPS = std::stoi(readValue(i, arg));
                supplied.insert("TargetFPS");
            } else if (arg == "--window-width") {
                cli.windowWidth = std::stoi(readValue(i, arg));
                supplied.insert("WindowWidth");
            } else if (arg == "--window-height") {
                cli.windowHeight = std::stoi(readValue(i, arg));
                supplied.insert("WindowHeight");
            } else if (arg == "--window-title") {
                cli.windowTitle = readValue(i, arg);
                supplied.insert("WindowTitle");
            } else if (arg == "--gamma") {
                cli.gamma = std::stof(readValue(i, arg));
                supplied.insert("Gamma");
            } else if (arg == "--vsync" || arg == "--no-vsync") {
                cli.vsync = (arg == "--vsync");
                supplied.insert("VSync");
            } else if (arg == "--msaa" || arg == "--no-msaa") {
                cli.msaa4x = (arg == "--msaa");
                supplied.insert("MSAA4x");
            } else if (arg == "--regen-atlas" || arg == "--no-regen-atlas") {
                cli.atlasRegeneration = (arg == "--regen-atlas");
                supplied.insert("AtlasRegeneration");
            } else if (arg == "-rl" || arg == "--raylib-log-level") {
                cli.raylibLogLevel = parseLogLevel(readValue(i, arg));
                supplied.insert("RaylibLogLevel");
            } else {
                throw std::runtime_error("argument inconnu : " + arg);
            }
        }

        Config config;
        if (!disableConfigFile) {
            if (!fs::exists(configPath) && !disableConfigFileAutoInit) {
                CSimpleIniA ini;
                ini.SetUnicode();
                ini.SetMultiKey(false);

                ini.SetValue("Settings", "GameDirectory", "./");
                ini.SetValue("Settings", "Username", "DefaultUser");
                ini.SetValue("Settings", "WindowWidth", "1500");
                ini.SetValue("Settings", "WindowHeight", "800");
                ini.SetValue("Settings", "TargetFPS", "60");
                ini.SetValue("Settings", "WindowTitle", "Nyx");
                ini.SetValue("Settings", "RenderDistance", "2");
                ini.SetValue("Settings", "Gamma", "2.2");
                ini.SetValue("Settings", "Seed", "1337");
                ini.SetValue("Settings", "AtlasRegeneration", "false");
                ini.SetValue("Settings", "VSync", "false");
                ini.SetValue("Settings", "MSAA4x", "false");
                ini.SetValue("Settings", "RaylibLogLevel", "INFO");
                ini.SaveFile(configPath.c_str());
            }

            if (fs::exists(configPath)) {
                config = readConfigFile(configPath);
            }
        }

        if (supplied.contains("GameDirectory")) config.gameDirectory = cli.gameDirectory;
        if (supplied.contains("Username")) config.username = cli.username;
        if (supplied.contains("RenderDistance")) config.renderDistance = cli.renderDistance;
        if (supplied.contains("Seed")) config.seed = cli.seed;
        if (supplied.contains("TargetFPS")) config.targetFPS = cli.targetFPS;
        if (supplied.contains("WindowWidth")) config.windowWidth = cli.windowWidth;
        if (supplied.contains("WindowHeight")) config.windowHeight = cli.windowHeight;
        if (supplied.contains("WindowTitle")) config.windowTitle = cli.windowTitle;
        if (supplied.contains("Gamma")) config.gamma = cli.gamma;
        if (supplied.contains("AtlasRegeneration")) {
            config.atlasRegeneration = cli.atlasRegeneration;
        }
        if (supplied.contains("VSync")) config.vsync = cli.vsync;
        if (supplied.contains("MSAA4x")) config.msaa4x = cli.msaa4x;
        if (supplied.contains("RaylibLogLevel")) {
            config.raylibLogLevel = cli.raylibLogLevel;
        }

        if (config.gameDirectory.empty()) {
            throw std::runtime_error("--game-dir est requis, ou doit être défini dans nyx.ini");
        }
        if (config.renderDistance <= 0 || config.targetFPS <= 0 ||
            config.windowWidth <= 0 || config.windowHeight <= 0 ||
            config.gamma <= 0.0f) {
            throw std::runtime_error(
                "render distance, FPS, dimensions et gamma doivent être positifs");
        }

        result.config = std::move(config);
        return result;
    } catch (const std::exception& e) {
        printUsage(argv[0]);
        throw std::runtime_error(e.what());
    }
}
