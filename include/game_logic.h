#ifndef CHIPS_GAME_LOGIC_H
#define CHIPS_GAME_LOGIC_H

#include "chips_types.h"

void game_init(GameState *state, const char *dat_path);
void game_load_level(GameState *state, int level_num);
void game_restart_level(GameState *state);
void game_next_level(GameState *state);
void game_prev_level(GameState *state);
bool game_goto_password(GameState *state, const char *pass);
void game_move_player(GameState *state, Direction dir);
void game_update_timer(GameState *state);
void game_update_sliding(GameState *state);
void game_update_creatures(GameState *state);

#endif /* CHIPS_GAME_LOGIC_H */
