#ifndef CHIPS_TYPES_H
#define CHIPS_TYPES_H

#include <stdint.h>
#include <stdbool.h>

#define GRID_W 32
#define GRID_H 32
#define VIEW_TILES 9
#define TILE_SIZE 32
#define VIEW_SIZE (VIEW_TILES * TILE_SIZE) /* 288 x 288 px */

/* Direct DAT File Object Codes (from CHIPS.DAT format) */
enum TileFileCode {
    FC_EMPTY            = 0x00,
    FC_WALL             = 0x01,
    FC_CHIP             = 0x02,
    FC_WATER            = 0x03,
    FC_FIRE             = 0x04,
    FC_HIDDEN_WALL_PERM = 0x05,
    FC_WALL_NORTH       = 0x06,
    FC_WALL_WEST        = 0x07,
    FC_WALL_SOUTH       = 0x08,
    FC_WALL_EAST        = 0x09,
    FC_BLOCK_STATIC     = 0x0A,
    FC_DIRT             = 0x0B,
    FC_ICE              = 0x0C,
    FC_SLIDE_SOUTH      = 0x0D,
    FC_CLONE_BLOCK_N    = 0x0E,
    FC_CLONE_BLOCK_W    = 0x0F,
    FC_CLONE_BLOCK_S    = 0x10,
    FC_CLONE_BLOCK_E    = 0x11,
    FC_SLIDE_NORTH      = 0x12,
    FC_SLIDE_EAST       = 0x13,
    FC_SLIDE_WEST       = 0x14,
    FC_EXIT             = 0x15,
    FC_DOOR_BLUE        = 0x16,
    FC_DOOR_RED         = 0x17,
    FC_DOOR_GREEN       = 0x18,
    FC_DOOR_YELLOW      = 0x19,
    FC_ICE_SE           = 0x1A,
    FC_ICE_SW           = 0x1B,
    FC_ICE_NW           = 0x1C,
    FC_ICE_NE           = 0x1D,
    FC_BLUE_WALL_FAKE   = 0x1E,
    FC_BLUE_WALL_REAL   = 0x1F,
    FC_BURGLAR          = 0x21,
    FC_SOCKET           = 0x22,
    FC_BUTTON_GREEN     = 0x23,
    FC_BUTTON_RED       = 0x24,
    FC_SWITCH_CLOSED    = 0x25,
    FC_SWITCH_OPEN      = 0x26,
    FC_BUTTON_BROWN     = 0x27,
    FC_BUTTON_BLUE      = 0x28,
    FC_TELEPORT         = 0x29,
    FC_BOMB             = 0x2A,
    FC_TRAP             = 0x2B,
    FC_HIDDEN_WALL_TEMP = 0x2C,
    FC_GRAVEL           = 0x2D,
    FC_POPUP_WALL       = 0x2E,
    FC_HINT             = 0x2F,
    FC_WALL_SE          = 0x30,
    FC_CLONE_MACHINE    = 0x31,
    FC_SLIDE_RANDOM     = 0x32,
    FC_DROWNED_CHIP     = 0x33,
    FC_BURNED_CHIP      = 0x34,
    FC_BOMBED_CHIP      = 0x35,
    FC_EXITED_CHIP      = 0x39,
    FC_SWIM_CHIP_N      = 0x3C,
    FC_SWIM_CHIP_W      = 0x3D,
    FC_SWIM_CHIP_S      = 0x3E,
    FC_SWIM_CHIP_E      = 0x3F,
    FC_BUG_N            = 0x40,
    FC_BUG_W            = 0x41,
    FC_BUG_S            = 0x42,
    FC_BUG_E            = 0x43,
    FC_FIREBALL_N       = 0x44,
    FC_FIREBALL_W       = 0x45,
    FC_FIREBALL_S       = 0x46,
    FC_FIREBALL_E       = 0x47,
    FC_BALL_N           = 0x48,
    FC_BALL_W           = 0x49,
    FC_BALL_S           = 0x4A,
    FC_BALL_E           = 0x4B,
    FC_TANK_N           = 0x4C,
    FC_TANK_W           = 0x4D,
    FC_TANK_S           = 0x4E,
    FC_TANK_E           = 0x4F,
    FC_GLIDER_N         = 0x50,
    FC_GLIDER_W         = 0x51,
    FC_GLIDER_S         = 0x52,
    FC_GLIDER_E         = 0x53,
    FC_TEETH_N          = 0x54,
    FC_TEETH_W          = 0x55,
    FC_TEETH_S          = 0x56,
    FC_TEETH_E          = 0x57,
    FC_WALKER_N         = 0x58,
    FC_WALKER_W         = 0x59,
    FC_WALKER_S         = 0x5A,
    FC_WALKER_E         = 0x5B,
    FC_BLOB_N           = 0x5C,
    FC_BLOB_W           = 0x5D,
    FC_BLOB_S           = 0x5E,
    FC_BLOB_E           = 0x5F,
    FC_PARAMECIUM_N     = 0x60,
    FC_PARAMECIUM_W     = 0x61,
    FC_PARAMECIUM_S     = 0x62,
    FC_PARAMECIUM_E     = 0x63,
    FC_KEY_BLUE         = 0x64,
    FC_KEY_RED          = 0x65,
    FC_KEY_GREEN        = 0x66,
    FC_KEY_YELLOW       = 0x67,
    FC_BOOTS_WATER      = 0x68,
    FC_BOOTS_FIRE       = 0x69,
    FC_BOOTS_ICE        = 0x6A,
    FC_BOOTS_SLIDE      = 0x6B,
    FC_CHIP_N           = 0x6C,
    FC_CHIP_W           = 0x6D,
    FC_CHIP_S           = 0x6E,
    FC_CHIP_E           = 0x6F
};

typedef enum Direction {
    DIR_NORTH = 0,
    DIR_WEST  = 1,
    DIR_SOUTH = 2,
    DIR_EAST  = 3,
    DIR_NONE  = 4
} Direction;

typedef struct Inventory {
    int keys_blue;
    int keys_red;
    int keys_green;
    int keys_yellow;
    bool has_flippers;
    bool has_fireboots;
    bool has_iceskates;
    bool has_suctionboots;
} Inventory;

typedef struct Coord {
    int x;
    int y;
} Coord;

typedef struct Creature {
    uint8_t base_species; /* FC_BUG_N (0x40), FC_FIREBALL_N (0x44), etc. */
    int x;
    int y;
    Direction dir;
    bool alive;
} Creature;

#define MAX_CREATURES 128

typedef struct LevelData {
    int number;
    int time_limit;
    int chips_needed;
    char title[64];
    char password[8];
    char hint[512];
    uint8_t top_layer[GRID_H][GRID_W];
    uint8_t bottom_layer[GRID_H][GRID_W];
    uint8_t initial_top[GRID_H][GRID_W];
    uint8_t initial_bottom[GRID_H][GRID_W];
    int start_x;
    int start_y;
    Direction start_dir;
    int num_creature_coords;
    Coord creature_coords[MAX_CREATURES];
} LevelData;

typedef struct GameState {
    int current_level_num;
    int total_levels;
    LevelData level;
    
    int chip_x;
    int chip_y;
    Direction chip_dir;
    bool chip_alive;
    bool chip_swimming;
    bool level_completed;
    
    int chips_remaining;
    int time_remaining;
    int score;
    
    Inventory inventory;
    
    Direction sliding_dir;
    bool is_sliding;
    
    Creature creatures[MAX_CREATURES];
    int num_creatures;
    int creature_tick_count;
    
    bool show_hint;
    char status_message[128];
    int status_timer;
} GameState;

#endif /* CHIPS_TYPES_H */
