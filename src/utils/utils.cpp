#include <SimpleIni.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
#include "../lib/Config.hpp"

namespace fs = std::filesystem;

bool initDefaultConfigFile(const std::string& filename) {
    try {
        if (fs::exists(filename)) return true;

        CSimpleIniA ini;
        ini.SetUnicode();
        ini.SetMultiKey(false);

        ini.SetValue("Settings", "WindowWidth", "800");
        ini.SetValue("Settings", "WindowHeight", "600");
        ini.SetValue("Settings", "TargetFPS", "60");
        ini.SetValue("Settings", "WindowTitle", "Nyx");
        ini.SetValue("Settings", "Gamma", "2.2");

        SI_Error rc = ini.SaveFile(filename.c_str());
        if (rc < 0) {
            std::cerr << "Warning: failed to create default config file '" << filename << "'\n";
            return false;
        }
        return true;
    } catch (const std::exception &e) {
        std::cerr << "Exception while creating default config file: " << e.what() << '\n';
        return false;
    } catch (...) {
        std::cerr << "Unknown error while creating default config file\n";
        return false;
    }
}

Config readConfig(const std::string& filename) {
    Config config;

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetMultiKey(false);

    if (ini.LoadFile(filename.c_str()) < 0) {
        throw std::runtime_error("failed to load config file");
    }

    auto getInt = [&](const char* section, const char* key, int fallback) -> int {
        const char* v = ini.GetValue(section, key, nullptr);
        if (!v) return fallback;
        try { return std::stoi(v); } catch (...) { return fallback; }
    };
    auto getFloat = [&](const char* section, const char* key, float fallback) -> float {
        const char* v = ini.GetValue(section, key, nullptr);
        if (!v) return fallback;
        try { return std::stof(v); } catch (...) { return fallback; }
    };
    auto getString = [&](const char* section, const char* key, const std::string &fallback) -> std::string {
        const char* v = ini.GetValue(section, key, nullptr);
        if (!v) return fallback;
        return std::string(v);
    };

    config.windowWidth = getInt("Settings", "WindowWidth", config.windowWidth);
    config.windowHeight = getInt("Settings", "WindowHeight", config.windowHeight);
    config.targetFPS = getInt("Settings", "TargetFPS", config.targetFPS);
    config.windowTitle = getString("Settings", "WindowTitle", config.windowTitle);
    config.gamma = getFloat("Settings", "Gamma", config.gamma);

    return config;
}

std::string genPath(const std::string& firstPath, const std::string& secondPath) { // generate a path from two others
    if (!firstPath.empty() && firstPath.back() != '/') {
        return firstPath + '/' + secondPath;
    }
    return firstPath + secondPath;
}