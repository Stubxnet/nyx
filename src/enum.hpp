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

// entity type
enum EntityType {
    PLAYER,
    MOB_RANGED,
    MOB_MELEE,
    MOB_OTHER,
    MOB_PASSIVE,
    TECHNICAL
};

// moving mode (for entities)
enum MoveMode {
    WALKING,
    FLYING,
    CREATIVE_FLY,
    SWIMMING,
    STATIC
};

// block types (actually only full but we will add "STAIRS", "SLAB" etc)
enum BlockType {
    FULL
};

// tag for chunk loading
enum class ChunkTag {
    NONE,
    ALWAYS_LOADED
};
