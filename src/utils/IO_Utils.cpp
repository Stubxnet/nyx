#pragma once

#include <raylib.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>

namespace fs = std::filesystem;

namespace IOutils {

static inline std::string readTextFile(const fs::path& p) {
    std::ifstream f(p);
    if (!f) throw std::runtime_error("Cannot open: " + p.string());
    return std::string((std::istreambuf_iterator<char>(f)), std::istreambuf_iterator<char>());
}

constexpr int TILE_SIZE = 32;

static inline void ensureTileSize(Image& img) {
    if (img.width == TILE_SIZE && img.height == TILE_SIZE) return;
    ImageResize(&img, TILE_SIZE, TILE_SIZE);
}

static inline Texture2D loadTextureOrFallback(const fs::path& p, bool* outIsFallback = nullptr) {
    bool fallback = false;

    if (!fs::exists(p)) {
        fallback = true;
        Image img = GenImageColor(TILE_SIZE, TILE_SIZE, MAGENTA);
        Texture2D t = LoadTextureFromImage(img);
        UnloadImage(img);
        if (outIsFallback) *outIsFallback = true;
        return t;
    }

    Image img = LoadImage(p.string().c_str());
    if (img.width == 0 || img.height == 0) {
        UnloadImage(img);
        fallback = true;
        Image fb = GenImageColor(TILE_SIZE, TILE_SIZE, MAGENTA);
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
    if (!fs::exists(p)) {
        return GenImageColor(TILE_SIZE, TILE_SIZE, MAGENTA);
    }
    Image img = LoadImage(p.string().c_str());
    if (img.width == 0 || img.height == 0) {
        UnloadImage(img);
        return GenImageColor(TILE_SIZE, TILE_SIZE, MAGENTA);
    }
    ensureTileSize(img);
    return img;
}

bool CheckMousePosition(int buttonX, int buttonY, int buttonWidth, int buttonHeight) {
    Vector2 mousePoint = { (float)GetMouseX(), (float)GetMouseY() };

    return (mousePoint.x >= buttonX && mousePoint.x <= buttonX + buttonWidth && mousePoint.y >= buttonY && mousePoint.y <= buttonY + buttonHeight);
}


}
