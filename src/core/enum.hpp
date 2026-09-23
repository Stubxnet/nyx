#pragma once

// This file contains the different "enum" types used.

//---------------------------Game menu--------------------------
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

//-----------------------------World--------------------------------------
///////// Blocks ///////////
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

// block types
enum class BlockType : uint8_t {
    FULL,
    STAIRS,
    SLAB,
    ORIENTED,
    LEAVES
};

// represents the block material (used for rendering and for gameplay)
enum BlockMaterial {
    DIRT,
    STONE,
    WOOD,
    VEGETAL
};

/////////// Chunks ////////////
// tag for chunk loading
enum class ChunkState : uint8_t {
    Unloaded,
    Generated,
    Dirty,
    Meshing,
    Ready,
    Unloading
};

//-----------------------------Entities/Player-------------------------------
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
