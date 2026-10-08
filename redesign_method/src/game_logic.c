#include "game_logic.h"
#include "dat_loader.h"
#include "audio.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

static void get_dir_delta(Direction dir, int *dx, int *dy) {
    *dx = 0;
    *dy = 0;
    switch (dir) {
        case DIR_NORTH: *dy = -1; break;
        case DIR_WEST:  *dx = -1; break;
        case DIR_SOUTH: *dy =  1; break;
        case DIR_EAST:  *dx =  1; break;
        default: break;
    }
}

void game_init(GameState *state, const char *dat_path) {
    memset(state, 0, sizeof(GameState));
    dat_init(dat_path);
    state->total_levels = dat_get_level_count();
    game_load_level(state, 1);
}

void game_load_level(GameState *state, int level_num) {
    if (level_num < 1) level_num = 1;
    if (level_num > state->total_levels) level_num = state->total_levels;

    state->current_level_num = level_num;
    dat_load_level(level_num, &state->level);

    state->chip_x = state->level.start_x;
    state->chip_y = state->level.start_y;
    state->chip_dir = state->level.start_dir;
    state->chip_alive = true;
    state->chip_swimming = false;
    state->level_completed = false;

    state->chips_remaining = state->level.chips_needed;
    state->time_remaining = state->level.time_limit;
    state->is_sliding = false;
    state->sliding_dir = DIR_NONE;
    state->show_hint = false;
    state->status_message[0] = '\0';

    memset(&state->inventory, 0, sizeof(Inventory));

    /* Remove player tile from top layer so it doesn't duplicate */
    state->level.top_layer[state->chip_y][state->chip_x] = FC_EMPTY;

    /* Initialize creatures */
    state->num_creatures = 0;
    state->creature_tick_count = 0;

    /* 1. Add creatures from Field 10 moving order list first */
    for (int i = 0; i < state->level.num_creature_coords; ++i) {
        int cx = state->level.creature_coords[i].x;
        int cy = state->level.creature_coords[i].y;
        if (cx >= 0 && cx < GRID_W && cy >= 0 && cy < GRID_H) {
            uint8_t tile = state->level.top_layer[cy][cx];
            if (tile >= FC_BUG_N && tile <= FC_PARAMECIUM_E && state->num_creatures < MAX_CREATURES) {
                Creature *cr = &state->creatures[state->num_creatures++];
                cr->x = cx;
                cr->y = cy;
                cr->base_species = (tile / 4) * 4;
                cr->dir = (Direction)(tile % 4);
                cr->alive = true;
            }
        }
    }

    /* 2. Scan remainder of the map for any creatures not listed in Field 10 */
    for (int y = 0; y < GRID_H; ++y) {
        for (int x = 0; x < GRID_W; ++x) {
            uint8_t tile = state->level.top_layer[y][x];
            if (tile >= FC_BUG_N && tile <= FC_PARAMECIUM_E) {
                bool already_added = false;
                for (int k = 0; k < state->num_creatures; ++k) {
                    if (state->creatures[k].x == x && state->creatures[k].y == y) {
                        already_added = true;
                        break;
                    }
                }
                if (!already_added && state->num_creatures < MAX_CREATURES) {
                    Creature *cr = &state->creatures[state->num_creatures++];
                    cr->x = x;
                    cr->y = y;
                    cr->base_species = (tile / 4) * 4;
                    cr->dir = (Direction)(tile % 4);
                    cr->alive = true;
                }
            }
        }
    }

    audio_play_music(1);
}

void game_restart_level(GameState *state) {
    game_load_level(state, state->current_level_num);
}

void game_next_level(GameState *state) {
    if (state->current_level_num < state->total_levels) {
        game_load_level(state, state->current_level_num + 1);
    }
}

void game_prev_level(GameState *state) {
    if (state->current_level_num > 1) {
        game_load_level(state, state->current_level_num - 1);
    }
}

bool game_goto_password(GameState *state, const char *pass) {
    if (!pass || strlen(pass) == 0) return false;

    /* Skip leading whitespace */
    while (*pass == ' ') pass++;

    /* Check if user input is a level number (1..149) */
    char *endptr = NULL;
    long lvl_num = strtol(pass, &endptr, 10);
    if (endptr && (*endptr == '\0' || *endptr == ' ') && lvl_num >= 1 && lvl_num <= state->total_levels) {
        game_load_level(state, (int)lvl_num);
        return true;
    }

    /* Otherwise, search by 4-letter password */
    char target[5];
    int len = 0;
    for (int i = 0; pass[i] != '\0' && len < 4; ++i) {
        if (!isspace((unsigned char)pass[i])) {
            target[len++] = (char)toupper((unsigned char)pass[i]);
        }
    }
    target[len] = '\0';

    if (len < 4) {
        snprintf(state->status_message, sizeof(state->status_message), "Password harus 4 huruf!");
        return false;
    }

    LevelData test_lvl;
    for (int i = 1; i <= state->total_levels; ++i) {
        if (dat_load_level(i, &test_lvl)) {
            if (strncmp(test_lvl.password, target, 4) == 0) {
                game_load_level(state, i);
                return true;
            }
        }
    }

    snprintf(state->status_message, sizeof(state->status_message), "Password '%s' tidak valid!", target);
    return false;
}

static void toggle_switch_walls(GameState *state) {
    for (int y = 0; y < GRID_H; ++y) {
        for (int x = 0; x < GRID_W; ++x) {
            if (state->level.top_layer[y][x] == FC_SWITCH_CLOSED) {
                state->level.top_layer[y][x] = FC_SWITCH_OPEN;
            } else if (state->level.top_layer[y][x] == FC_SWITCH_OPEN) {
                state->level.top_layer[y][x] = FC_SWITCH_CLOSED;
            }
        }
    }
}

static void toggle_tanks(GameState *state) {
    for (int i = 0; i < state->num_creatures; ++i) {
        Creature *cr = &state->creatures[i];
        if (cr->alive && cr->base_species == FC_TANK_N) {
            cr->dir = (Direction)((cr->dir + 2) % 4);
            state->level.top_layer[cr->y][cr->x] = FC_TANK_N + cr->dir;
        }
    }
}

static void handle_teleport(GameState *state, int cur_x, int cur_y, Direction dir) {
    int dx, dy;
    get_dir_delta(dir, &dx, &dy);

    /* Find next teleport pad in reading order */
    int start_idx = cur_y * GRID_W + cur_x;
    for (int offset = 1; offset < GRID_W * GRID_H; ++offset) {
        int idx = (start_idx + offset) % (GRID_W * GRID_H);
        int ty = idx / GRID_W;
        int tx = idx % GRID_W;
        if (state->level.top_layer[ty][tx] == FC_TELEPORT || state->level.bottom_layer[ty][tx] == FC_TELEPORT) {
            int out_x = tx + dx;
            int out_y = ty + dy;
            if (out_x >= 0 && out_x < GRID_W && out_y >= 0 && out_y < GRID_H) {
                uint8_t dest_tile = state->level.top_layer[out_y][out_x];
                if (dest_tile == FC_EMPTY || dest_tile == FC_DIRT || dest_tile == FC_CHIP) {
                    state->chip_x = out_x;
                    state->chip_y = out_y;
                    audio_play_sfx(SFX_TELEPORT);
                    return;
                }
            }
            state->chip_x = tx;
            state->chip_y = ty;
            audio_play_sfx(SFX_TELEPORT);
            return;
        }
    }
}

void game_move_player(GameState *state, Direction dir) {
    if (!state->chip_alive || state->level_completed) return;

    state->chip_dir = dir;
    state->show_hint = false;

    int dx, dy;
    get_dir_delta(dir, &dx, &dy);

    int nx = state->chip_x + dx;
    int ny = state->chip_y + dy;

    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) {
        audio_play_sfx(SFX_BUMP);
        return;
    }

    uint8_t top = state->level.top_layer[ny][nx];
    uint8_t bottom = state->level.bottom_layer[ny][nx];

    /* Blocking obstacles */
    if (top == FC_WALL || top == FC_BLUE_WALL_REAL || top == FC_SWITCH_CLOSED ||
        top == FC_CLONE_MACHINE || top == FC_POPUP_WALL) {
        audio_play_sfx(SFX_BUMP);
        return;
    }

    /* Collision with monsters: walking into a monster is fatal */
    if (top >= FC_BUG_N && top <= FC_PARAMECIUM_E) {
        state->chip_alive = false;
        audio_play_sfx(SFX_DEATH);
        return;
    }

    /* Thin walls */
    if ((top == FC_WALL_NORTH && dir == DIR_SOUTH) ||
        (top == FC_WALL_SOUTH && dir == DIR_NORTH) ||
        (top == FC_WALL_WEST  && dir == DIR_EAST)  ||
        (top == FC_WALL_EAST  && dir == DIR_WEST)) {
        audio_play_sfx(SFX_BUMP);
        return;
    }

    /* Hidden walls */
    if (top == FC_HIDDEN_WALL_TEMP || top == FC_HIDDEN_WALL_PERM) {
        state->level.top_layer[ny][nx] = FC_WALL;
        audio_play_sfx(SFX_BUMP);
        return;
    }

    /* Movable dirt block */
    if (top == FC_BLOCK_STATIC) {
        int bx = nx + dx;
        int by = ny + dy;
        if (bx < 0 || bx >= GRID_W || by < 0 || by >= GRID_H) {
            audio_play_sfx(SFX_BUMP);
            return;
        }
        uint8_t block_dest_top = state->level.top_layer[by][bx];
        uint8_t block_dest_bot = state->level.bottom_layer[by][bx];

        /* Check if destination tile is water (in top or bottom layer) */
        if (block_dest_top == FC_WATER || block_dest_bot == FC_WATER) {
            /* Block sinks into water and turns it into dirt! */
            state->level.top_layer[by][bx] = FC_DIRT;
            state->level.bottom_layer[by][bx] = FC_EMPTY;

            /* Clear block from previous tile (restore under-tile if any) */
            if (state->level.bottom_layer[ny][nx] != FC_EMPTY) {
                state->level.top_layer[ny][nx] = state->level.bottom_layer[ny][nx];
                state->level.bottom_layer[ny][nx] = FC_EMPTY;
            } else {
                state->level.top_layer[ny][nx] = FC_EMPTY;
            }

            state->chip_x = nx;
            state->chip_y = ny;
            audio_play_sfx(SFX_WATER);
            return;
        } else if (block_dest_top == FC_BOMB || block_dest_bot == FC_BOMB) {
            /* Block detonates bomb */
            state->level.top_layer[by][bx] = FC_EMPTY;
            state->level.bottom_layer[by][bx] = FC_EMPTY;

            if (state->level.bottom_layer[ny][nx] != FC_EMPTY) {
                state->level.top_layer[ny][nx] = state->level.bottom_layer[ny][nx];
                state->level.bottom_layer[ny][nx] = FC_EMPTY;
            } else {
                state->level.top_layer[ny][nx] = FC_EMPTY;
            }

            state->chip_x = nx;
            state->chip_y = ny;
            audio_play_sfx(SFX_BOMB);
            return;
        } else if (block_dest_top == FC_EMPTY) {
            state->level.top_layer[by][bx] = FC_BLOCK_STATIC;

            if (state->level.bottom_layer[ny][nx] != FC_EMPTY) {
                state->level.top_layer[ny][nx] = state->level.bottom_layer[ny][nx];
                state->level.bottom_layer[ny][nx] = FC_EMPTY;
            } else {
                state->level.top_layer[ny][nx] = FC_EMPTY;
            }

            state->chip_x = nx;
            state->chip_y = ny;
            audio_play_sfx(SFX_MOVE);
            return;
        } else if (block_dest_top == FC_ICE) {
            /* Block pushed onto ice */
            state->level.bottom_layer[by][bx] = FC_ICE;
            state->level.top_layer[by][bx] = FC_BLOCK_STATIC;

            if (state->level.bottom_layer[ny][nx] != FC_EMPTY) {
                state->level.top_layer[ny][nx] = state->level.bottom_layer[ny][nx];
                state->level.bottom_layer[ny][nx] = FC_EMPTY;
            } else {
                state->level.top_layer[ny][nx] = FC_EMPTY;
            }

            state->chip_x = nx;
            state->chip_y = ny;
            audio_play_sfx(SFX_MOVE);
            return;
        } else if (block_dest_top == FC_BUTTON_BROWN || block_dest_top == FC_BUTTON_BLUE ||
                   block_dest_top == FC_BUTTON_GREEN || block_dest_top == FC_BUTTON_RED) {
            /* Block pushed onto a button */
            state->level.bottom_layer[by][bx] = block_dest_top;
            state->level.top_layer[by][bx] = FC_BLOCK_STATIC;

            if (state->level.bottom_layer[ny][nx] != FC_EMPTY) {
                state->level.top_layer[ny][nx] = state->level.bottom_layer[ny][nx];
                state->level.bottom_layer[ny][nx] = FC_EMPTY;
            } else {
                state->level.top_layer[ny][nx] = FC_EMPTY;
            }

            state->chip_x = nx;
            state->chip_y = ny;
            if (block_dest_top == FC_BUTTON_GREEN) {
                toggle_switch_walls(state);
            }
            audio_play_sfx(SFX_MOVE);
            return;
        } else {
            audio_play_sfx(SFX_BUMP);
            return;
        }
    }

    /* Doors & Keys */
    if (top == FC_DOOR_BLUE) {
        if (state->inventory.keys_blue > 0) {
            state->inventory.keys_blue--;
            state->level.top_layer[ny][nx] = FC_EMPTY;
            audio_play_sfx(SFX_OPEN_DOOR);
        } else {
            audio_play_sfx(SFX_BUMP);
            return;
        }
    } else if (top == FC_DOOR_RED) {
        if (state->inventory.keys_red > 0) {
            state->inventory.keys_red--;
            state->level.top_layer[ny][nx] = FC_EMPTY;
            audio_play_sfx(SFX_OPEN_DOOR);
        } else {
            audio_play_sfx(SFX_BUMP);
            return;
        }
    } else if (top == FC_DOOR_GREEN) {
        if (state->inventory.keys_green > 0) {
            /* Green keys are not consumed! */
            state->level.top_layer[ny][nx] = FC_EMPTY;
            audio_play_sfx(SFX_OPEN_DOOR);
        } else {
            audio_play_sfx(SFX_BUMP);
            return;
        }
    } else if (top == FC_DOOR_YELLOW) {
        if (state->inventory.keys_yellow > 0) {
            state->inventory.keys_yellow--;
            state->level.top_layer[ny][nx] = FC_EMPTY;
            audio_play_sfx(SFX_OPEN_DOOR);
        } else {
            audio_play_sfx(SFX_BUMP);
            return;
        }
    }

    /* Chip Socket */
    if (top == FC_SOCKET) {
        if (state->chips_remaining <= 0) {
            state->level.top_layer[ny][nx] = FC_EMPTY;
            audio_play_sfx(SFX_SOCKET_OPEN);
        } else {
            snprintf(state->status_message, sizeof(state->status_message), "You still need %d chips!", state->chips_remaining);
            audio_play_sfx(SFX_BUMP);
            return;
        }
    }

    /* Move player to new position */
    state->chip_x = nx;
    state->chip_y = ny;

    /* Items pick up */
    if (top == FC_CHIP) {
        if (state->chips_remaining > 0) state->chips_remaining--;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_CHIP);
    } else if (top == FC_KEY_BLUE) {
        state->inventory.keys_blue++;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_KEY);
    } else if (top == FC_KEY_RED) {
        state->inventory.keys_red++;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_KEY);
    } else if (top == FC_KEY_GREEN) {
        state->inventory.keys_green++;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_KEY);
    } else if (top == FC_KEY_YELLOW) {
        state->inventory.keys_yellow++;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_KEY);
    } else if (top == FC_BOOTS_WATER) {
        state->inventory.has_flippers = true;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_BOOTS);
    } else if (top == FC_BOOTS_FIRE) {
        state->inventory.has_fireboots = true;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_BOOTS);
    } else if (top == FC_BOOTS_ICE) {
        state->inventory.has_iceskates = true;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_BOOTS);
    } else if (top == FC_BOOTS_SLIDE) {
        state->inventory.has_suctionboots = true;
        state->level.top_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_PICKUP_BOOTS);
    } else if (top == FC_DIRT) {
        state->level.top_layer[ny][nx] = FC_EMPTY;
        state->level.bottom_layer[ny][nx] = FC_EMPTY;
        audio_play_sfx(SFX_MOVE);
    } else if (top == FC_HINT) {
        state->show_hint = true;
        audio_play_sfx(SFX_MOVE);
    } else if (top == FC_BUTTON_GREEN) {
        toggle_switch_walls(state);
        audio_play_sfx(SFX_MOVE);
    } else if (top == FC_BUTTON_BLUE) {
        toggle_tanks(state);
        audio_play_sfx(SFX_MOVE);
    } else if (top == FC_BURGLAR) {
        /* Thief takes all boots */
        state->inventory.has_flippers = false;
        state->inventory.has_fireboots = false;
        state->inventory.has_iceskates = false;
        state->inventory.has_suctionboots = false;
        audio_play_sfx(SFX_MOVE);
    } else if (top == FC_TELEPORT || bottom == FC_TELEPORT) {
        handle_teleport(state, nx, ny, dir);
        return;
    } else {
        audio_play_sfx(SFX_MOVE);
    }

    /* Environmental hazards (Water, Fire, Bomb) */
    uint8_t effective_floor = (top == FC_WATER || top == FC_FIRE || top == FC_BOMB || top == FC_EXIT) ? top : bottom;
    if (effective_floor == FC_WATER) {
        if (!state->inventory.has_flippers) {
            state->chip_alive = false;
            audio_play_sfx(SFX_WATER);
            audio_play_sfx(SFX_DEATH);
            return;
        }
    } else if (effective_floor == FC_FIRE) {
        if (!state->inventory.has_fireboots) {
            state->chip_alive = false;
            audio_play_sfx(SFX_DEATH);
            return;
        }
    } else if (effective_floor == FC_BOMB) {
        state->chip_alive = false;
        audio_play_sfx(SFX_BOMB);
        audio_play_sfx(SFX_DEATH);
        return;
    } else if (effective_floor == FC_EXIT) {
        state->level_completed = true;
        audio_play_sfx(SFX_WIN);
        return;
    }

    /* Ice & Force floor triggers */
    if ((effective_floor == FC_ICE && !state->inventory.has_iceskates) ||
        (effective_floor >= FC_ICE_SE && effective_floor <= FC_ICE_NE && !state->inventory.has_iceskates)) {
        state->is_sliding = true;
        if (effective_floor == FC_ICE_SE) {
            state->sliding_dir = (dir == DIR_NORTH) ? DIR_EAST : ((dir == DIR_WEST) ? DIR_SOUTH : dir);
        } else if (effective_floor == FC_ICE_SW) {
            state->sliding_dir = (dir == DIR_NORTH) ? DIR_WEST : ((dir == DIR_EAST) ? DIR_SOUTH : dir);
        } else if (effective_floor == FC_ICE_NW) {
            state->sliding_dir = (dir == DIR_SOUTH) ? DIR_WEST : ((dir == DIR_EAST) ? DIR_NORTH : dir);
        } else if (effective_floor == FC_ICE_NE) {
            state->sliding_dir = (dir == DIR_SOUTH) ? DIR_EAST : ((dir == DIR_WEST) ? DIR_NORTH : dir);
        } else {
            state->sliding_dir = dir;
        }
    } else if (!state->inventory.has_suctionboots &&
               (effective_floor == FC_SLIDE_NORTH || effective_floor == FC_SLIDE_SOUTH ||
                effective_floor == FC_SLIDE_EAST  || effective_floor == FC_SLIDE_WEST)) {
        state->is_sliding = true;
        if (effective_floor == FC_SLIDE_NORTH) state->sliding_dir = DIR_NORTH;
        else if (effective_floor == FC_SLIDE_SOUTH) state->sliding_dir = DIR_SOUTH;
        else if (effective_floor == FC_SLIDE_EAST) state->sliding_dir = DIR_EAST;
        else if (effective_floor == FC_SLIDE_WEST) state->sliding_dir = DIR_WEST;
    } else {
        state->is_sliding = false;
        state->sliding_dir = DIR_NONE;
    }
}

void game_update_timer(GameState *state) {
    if (!state->chip_alive || state->level_completed) return;

    if (state->level.time_limit > 0) {
        state->time_remaining--;
        if (state->time_remaining <= 0) {
            state->time_remaining = 0;
            state->chip_alive = false;
            audio_play_sfx(SFX_DEATH);
        }
    }
}

void game_update_sliding(GameState *state) {
    if (state->is_sliding && state->chip_alive && !state->level_completed) {
        game_move_player(state, state->sliding_dir);
    }
}

static Direction dir_left(Direction dir) {
    return (Direction)((dir + 1) % 4);
}
static Direction dir_right(Direction dir) {
    return (Direction)((dir + 3) % 4);
}
static Direction dir_back(Direction dir) {
    return (Direction)((dir + 2) % 4);
}

static bool can_creature_enter(const GameState *state, const Creature *cr, int nx, int ny) {
    if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) return false;

    uint8_t top = state->level.top_layer[ny][nx];

    /* Impassable walls and obstacles */
    if (top == FC_WALL || top == FC_BLUE_WALL_REAL || top == FC_BLUE_WALL_FAKE ||
        top == FC_HIDDEN_WALL_TEMP || top == FC_HIDDEN_WALL_PERM ||
        top == FC_SWITCH_CLOSED || top == FC_CLONE_MACHINE ||
        top == FC_POPUP_WALL || top == FC_BLOCK_STATIC || top == FC_DIRT ||
        top == FC_GRAVEL || top == FC_SOCKET || top == FC_EXIT) {
        return false;
    }

    /* Thin walls */
    if ((top == FC_WALL_NORTH && cr->dir == DIR_SOUTH) ||
        (top == FC_WALL_SOUTH && cr->dir == DIR_NORTH) ||
        (top == FC_WALL_WEST  && cr->dir == DIR_EAST)  ||
        (top == FC_WALL_EAST  && cr->dir == DIR_WEST)) {
        return false;
    }

    /* Doors and inventory items block creatures */
    if (top >= FC_DOOR_BLUE && top <= FC_DOOR_YELLOW) return false;
    if (top == FC_CHIP) return false;
    if (top >= FC_KEY_BLUE && top <= FC_BOOTS_SLIDE) return false;

    /* Check if another alive creature is already on that tile */
    for (int i = 0; i < state->num_creatures; ++i) {
        const Creature *other = &state->creatures[i];
        if (other != cr && other->alive && other->x == nx && other->y == ny) {
            return false;
        }
    }

    return true;
}

void game_update_creatures(GameState *state) {
    if (!state->chip_alive || state->level_completed) return;

    state->creature_tick_count++;

    for (int i = 0; i < state->num_creatures; ++i) {
        Creature *cr = &state->creatures[i];
        if (!cr->alive) continue;

        /* Teeth and Blob move at half speed (every 2 creature ticks) */
        if ((cr->base_species == FC_TEETH_N || cr->base_species == FC_BLOB_N) &&
            (state->creature_tick_count % 2) != 0) {
            continue;
        }

        Direction choices[4] = { DIR_NONE, DIR_NONE, DIR_NONE, DIR_NONE };
        int num_choices = 0;

        switch (cr->base_species) {
            case FC_BUG_N: /* Left-edge follower */
                choices[0] = dir_left(cr->dir);
                choices[1] = cr->dir;
                choices[2] = dir_right(cr->dir);
                choices[3] = dir_back(cr->dir);
                num_choices = 4;
                break;

            case FC_PARAMECIUM_N: /* Right-edge follower */
                choices[0] = dir_right(cr->dir);
                choices[1] = cr->dir;
                choices[2] = dir_left(cr->dir);
                choices[3] = dir_back(cr->dir);
                num_choices = 4;
                break;

            case FC_FIREBALL_N: /* Straight, right, left, back */
                choices[0] = cr->dir;
                choices[1] = dir_right(cr->dir);
                choices[2] = dir_left(cr->dir);
                choices[3] = dir_back(cr->dir);
                num_choices = 4;
                break;

            case FC_GLIDER_N: /* Straight, left, right, back */
                choices[0] = cr->dir;
                choices[1] = dir_left(cr->dir);
                choices[2] = dir_right(cr->dir);
                choices[3] = dir_back(cr->dir);
                num_choices = 4;
                break;

            case FC_BALL_N: /* Straight, then reverse */
                choices[0] = cr->dir;
                choices[1] = dir_back(cr->dir);
                num_choices = 2;
                break;

            case FC_TANK_N: /* Straight only */
                choices[0] = cr->dir;
                num_choices = 1;
                break;

            case FC_TEETH_N: { /* Chase Chip */
                int dx = state->chip_x - cr->x;
                int dy = state->chip_y - cr->y;
                Direction h_dir = (dx > 0) ? DIR_EAST : ((dx < 0) ? DIR_WEST : DIR_NONE);
                Direction v_dir = (dy > 0) ? DIR_SOUTH : ((dy < 0) ? DIR_NORTH : DIR_NONE);
                int abs_x = (dx < 0) ? -dx : dx;
                int abs_y = (dy < 0) ? -dy : dy;
                if (abs_x >= abs_y) {
                    if (h_dir != DIR_NONE) choices[num_choices++] = h_dir;
                    if (v_dir != DIR_NONE) choices[num_choices++] = v_dir;
                } else {
                    if (v_dir != DIR_NONE) choices[num_choices++] = v_dir;
                    if (h_dir != DIR_NONE) choices[num_choices++] = h_dir;
                }
                break;
            }

            case FC_WALKER_N: { /* Straight, else random */
                choices[0] = cr->dir;
                Direction others[3] = { dir_left(cr->dir), dir_right(cr->dir), dir_back(cr->dir) };
                for (int k = 2; k > 0; --k) {
                    int r = rand() % (k + 1);
                    Direction tmp = others[k]; others[k] = others[r]; others[r] = tmp;
                }
                choices[1] = others[0];
                choices[2] = others[1];
                choices[3] = others[2];
                num_choices = 4;
                break;
            }

            case FC_BLOB_N: { /* Random all 4 */
                Direction all_dirs[4] = { DIR_NORTH, DIR_WEST, DIR_SOUTH, DIR_EAST };
                for (int k = 3; k > 0; --k) {
                    int r = rand() % (k + 1);
                    Direction tmp = all_dirs[k]; all_dirs[k] = all_dirs[r]; all_dirs[r] = tmp;
                }
                for (int k = 0; k < 4; ++k) choices[k] = all_dirs[k];
                num_choices = 4;
                break;
            }

            default:
                choices[0] = cr->dir;
                num_choices = 1;
                break;
        }

        /* Test choices */
        bool moved = false;
        for (int c = 0; c < num_choices; ++c) {
            Direction test_dir = choices[c];
            if (test_dir == DIR_NONE) continue;

            int cdx, cdy;
            get_dir_delta(test_dir, &cdx, &cdy);
            int nx = cr->x + cdx;
            int ny = cr->y + cdy;

            if (can_creature_enter(state, cr, nx, ny)) {
                int ox = cr->x;
                int oy = cr->y;

                /* Restore under-tile at previous position */
                if (state->level.bottom_layer[oy][ox] != FC_EMPTY) {
                    state->level.top_layer[oy][ox] = state->level.bottom_layer[oy][ox];
                    state->level.bottom_layer[oy][ox] = FC_EMPTY;
                } else {
                    state->level.top_layer[oy][ox] = FC_EMPTY;
                }

                cr->dir = test_dir;
                cr->x = nx;
                cr->y = ny;

                /* Check collision with Chip */
                if (nx == state->chip_x && ny == state->chip_y) {
                    state->chip_alive = false;
                    audio_play_sfx(SFX_DEATH);
                    return;
                }

                uint8_t dest_top = state->level.top_layer[ny][nx];
                uint8_t dest_bot = state->level.bottom_layer[ny][nx];

                /* Hazard interactions */
                if (dest_top == FC_WATER || dest_bot == FC_WATER) {
                    if (cr->base_species == FC_GLIDER_N) {
                        state->level.bottom_layer[ny][nx] = FC_WATER;
                        state->level.top_layer[ny][nx] = cr->base_species + cr->dir;
                    } else {
                        cr->alive = false;
                        audio_play_sfx(SFX_WATER);
                    }
                } else if (dest_top == FC_FIRE || dest_bot == FC_FIRE) {
                    if (cr->base_species == FC_FIREBALL_N) {
                        state->level.bottom_layer[ny][nx] = FC_FIRE;
                        state->level.top_layer[ny][nx] = cr->base_species + cr->dir;
                    } else {
                        cr->alive = false;
                    }
                } else if (dest_top == FC_BOMB || dest_bot == FC_BOMB) {
                    state->level.top_layer[ny][nx] = FC_EMPTY;
                    state->level.bottom_layer[ny][nx] = FC_EMPTY;
                    cr->alive = false;
                    audio_play_sfx(SFX_BOMB);
                } else {
                    /* Buttons, Ice, Sliders or normal floor */
                    if (dest_top == FC_BUTTON_GREEN) {
                        state->level.bottom_layer[ny][nx] = FC_BUTTON_GREEN;
                        toggle_switch_walls(state);
                    } else if (dest_top == FC_BUTTON_BLUE) {
                        state->level.bottom_layer[ny][nx] = FC_BUTTON_BLUE;
                        toggle_tanks(state);
                    } else if (dest_top == FC_BUTTON_BROWN || dest_top == FC_BUTTON_RED ||
                               dest_top == FC_ICE || (dest_top >= FC_SLIDE_SOUTH && dest_top <= FC_SLIDE_WEST)) {
                        state->level.bottom_layer[ny][nx] = dest_top;
                    }
                    state->level.top_layer[ny][nx] = cr->base_species + cr->dir;
                }

                moved = true;
                break;
            }
        }

        if (!moved && num_choices > 0 && choices[0] != DIR_NONE) {
            /* Creature could not move; turn in place towards preferred direction */
            cr->dir = choices[0];
            state->level.top_layer[cr->y][cr->x] = cr->base_species + cr->dir;
        }
    }
}
