#pragma once

// stores the active game screen
enum GameScreen { 
    MENU, 
    GAME, 
    OPTIONS,
    DEATH
};

// Represents the action of player in the MENU screen
enum class MenuAction { 
    NONE, 
    PLAY, 
    OPTIONS, 
    QUIT 
};

// Represents the action of player in the OPTIONS screen
enum class OptionsAction { 
    NONE, 
    QUIT, 
    BACK 
};

// types of blocks placing actions
enum class SetblockActions {
    SET,
    REPLACE,
    KEEP,
    BREAK
};

// types of blocks filling actions
enum class BlockFillActions {
    SET,
    REPLACE,
    KEEP,
    BREAK,
    OUTLINE
};

// different available gamemodes
enum GameModes { 
    SURVIVAL,
    CREATIVE,
    SPECTATOR,
    BUILDER,
    UNKNOWN
};