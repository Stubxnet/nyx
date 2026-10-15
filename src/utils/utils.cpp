#pragma once

#include "../core/constants.hpp"

#include <raylib.h>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <string>

namespace fs = std::filesystem;

static inline std::string readTextFile(const fs::path& p) {
    std::ifstream f(p);
    if (!f) throw std::runtime_error("Cannot open: " + p.string());
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}


static inline void ensureTileSize(Image& img) {
    if (img.width == TILE && img.height == TILE) return;
    ImageResize(&img, TILE, TILE);
}

static inline Texture2D loadTextureOrFallback(const fs::path& p, bool* outIsFallback = nullptr) {
    bool fallback = false;

    if (!fs::exists(p)) {
        fallback = true;
        Image img = GenImageColor(TILE, TILE, MAGENTA);
        Texture2D t = LoadTextureFromImage(img);
        UnloadImage(img);
        if (outIsFallback) *outIsFallback = true;
        return t;
    }

    Image img = LoadImage(p.string().c_str());
    if (img.width == 0 || img.height == 0) {
        UnloadImage(img);
        fallback = true;
        Image fb = GenImageColor(TILE, TILE, MAGENTA);
        Texture2D t = LoadTextureFromImage(fb);
        UnloadImage(fb);
        if (outIsFallback) *outIsFallback = true;
        return t;
    }

    ensureTileSize(img);
    Texture2D t = LoadTextureFromImage(img);
    UnloadImage(img);

    if (outIsFallback) *outIsFallback = fallback;
    return t;
}

static inline Image loadImageOrFallback(const fs::path& p) {
    if (!fs::exists(p)) { return GenImageColor(TILE, TILE, MAGENTA); }

    Image img = LoadImage(p.string().c_str());
    if (img.width == 0 || img.height == 0) {
        UnloadImage(img);
        return GenImageColor(TILE, TILE, MAGENTA);
    }
    ensureTileSize(img);
    return img;
}

static bool CheckMousePosition(int buttonX, int buttonY, int buttonWidth, int buttonHeight) {
    Vector2 mousePoint = { (float)GetMouseX(), (float)GetMouseY() };

    return (mousePoint.x >= buttonX && mousePoint.x <= buttonX + buttonWidth && mousePoint.y >= buttonY && mousePoint.y <= buttonY + buttonHeight);
}

std::string genPath(const std::string& firstPath, const std::string& secondPath) {
    // generate a path from two others paths
    if (!firstPath.empty() && firstPath.back() != '/') {
        return firstPath + '/' + secondPath;
    }
    return firstPath + secondPath;
}
