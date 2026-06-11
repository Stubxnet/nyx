#pragma once
#include <string>
#include <memory>
#include "../lib/World.hpp"
#include "../lib/GameMode.hpp"
#include "raylib.h"

struct CommandContext {
    std::shared_ptr<Camera> camera;
    std::shared_ptr<World> world;
    int *renderDistance;
    GameModes *currentGamemode;
};

static BlockFillActions ParseFillAction(const std::string &s) {
    if (s == "set") return BlockFillActions::SET;
    if (s == "replace") return BlockFillActions::REPLACE;
    if (s == "keep") return BlockFillActions::KEEP;
    if (s == "break") return BlockFillActions::BREAK;
    if (s == "outline") return BlockFillActions::OUTLINE;
    return BlockFillActions::SET;
}

bool HandleCommand(const std::string &input, CommandContext &ctx) {
    std::istringstream iss(input);
    std::string command;
    if (!(iss >> command)) return false;

    if (command == "teleport" || command == "tp") {
        float x,y,z;
        if (!(iss >> x >> y >> z)) { std::cout<<"Invalid teleport command.\n"; return false; }
        ctx.camera->position = { x,y,z };
        return true;
    }

    if (command == "rotation" || command == "rt") {
        float x,y,z;
        if (!(iss >> x >> y >> z)) { std::cout<<"Invalid rotation command.\n"; return false; }
        ctx.camera->target = { x,y,z };
        return true;
    }

    if (command == "fov") {
        float fov;
        if (!(iss >> fov)) { std::cout<<"Invalid fov command.\n"; return false; }
        ctx.camera->fovy = fov;
        return true;
    }

    if (command == "renderdistance" || command == "rd") {
        int rd;
        if (!(iss >> rd)) { std::cout<<"Invalid renderdistance command.\n"; return false; }
        if (ctx.renderDistance) *ctx.renderDistance = rd;
        return true;
    }

    if (command == "setblock" || command == "sb") {
        int x,y,z,id;
        if (!(iss >> x >> y >> z >> id)) { std::cout<<"Invalid setblock command.\n"; return false; }
        ctx.world->SetBlock(x,y,z,id);
        if (ctx.world->GetBlockId(x,y,z) != id)
            std::cout<<"Error placing block at "<<x<<","<<y<<","<<z<<"\n";
        else
            std::cout<<"Block successfully placed at "<<x<<","<<y<<","<<z<<"\n";
        return true;
    }

    if (command == "fill") {
        int x1,y1,z1,x2,y2,z2,id;
        std::string modeStr;
        if (!(iss >> x1 >> y1 >> z1 >> x2 >> y2 >> z2 >> id)) {
            std::cout<<"Invalid fill command. Usage: /fill x1 y1 z1 x2 y2 z2 id [replace|keep|break|outline|set]\n";
            return false;
        }
        if (!(iss >> modeStr)) modeStr = "set";
        std::transform(modeStr.begin(), modeStr.end(), modeStr.begin(), ::tolower);
        BlockFillActions action = ParseFillAction(modeStr);
        int placed = ctx.world->FillBlocks(x1,y1,z1,x2,y2,z2, action, id);
        if (placed > 0) std::cout<<"Filled area with id "<<id<<" (mode: "<<modeStr<<"). Blocks placed: "<<placed<<"\n";
        else std::cout<<"Failed to set blocks in the specified area.\n";
        return true;
    }

    if (command == "gamemode" || command == "gm") {
        int intMode;
        std::string stringMode;
        if (iss >> intMode) {
            if (ctx.currentGamemode) {
                switch (intMode) {
                    case 0: *ctx.currentGamemode = GameModes::SURVIVAL; break;
                    case 1: *ctx.currentGamemode = GameModes::CREATIVE; break;
                    case 2: *ctx.currentGamemode = GameModes::SPECTATOR; break;
                    default: *ctx.currentGamemode = GameModes::SURVIVAL; break;
                }
            }            return true;
        } else if (iss >> stringMode) {
            if (ctx.currentGamemode) {
                if (stringMode == "survival") *ctx.currentGamemode = GameModes::SURVIVAL;
                else if (stringMode == "creative") *ctx.currentGamemode = GameModes::CREATIVE;
                else if (stringMode == "spectator") *ctx.currentGamemode = GameModes::SPECTATOR;
                else *ctx.currentGamemode = GameModes::SURVIVAL;
            }
            return true;
        } else {
            std::cout << "Invalid gamemode command.\n";
            return false;
        }
    }

    std::cout<<"Unknown command.\n";
    return false;
}