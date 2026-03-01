#include <SimpleIni.h>
#include <iostream>
#include <string>
#include "lib/config.hpp"

Config readConfig(const std::string& filename) {
    Config config;

    CSimpleIniA ini;
    ini.SetUnicode();
    ini.SetMultiKey(false);

    // load nyx.ini
    if (ini.LoadFile(filename.c_str()) < 0) {
        std::cerr << "Error loading file " << filename << std::endl;
        return config; // returns default values
    }

    config.windowWidth = std::stoi(ini.GetValue("Settings", "WindowWidth", "800"));
    config.windowHeight = std::stoi(ini.GetValue("Settings", "WindowHeight", "600"));
    config.gameDirectory = ini.GetValue("Settings", "GameDirectory", "$HOME/.nyx/");
    config.username = ini.GetValue("Settings", "Username", "DefaultUser");
    config.renderDistance = std::stof(ini.GetValue("Settings", "RenderDistance", "1000"));
    config.gamma = std::stof(ini.GetValue("Settings", "Gamma", "2.2"));

    return config;
}

std::string genPath(const std::string& firstPath, const std::string& secondPath) { // generate a path from two others
    if (!firstPath.empty() && firstPath.back() != '/') {
        return firstPath + '/' + secondPath;
    }
    return firstPath + secondPath;
}