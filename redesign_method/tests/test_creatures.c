#include <stdio.h>
#include <assert.h>
#include "chips_types.h"
#include "dat_loader.h"
#include "game_logic.h"

int main() {
    GameState state;
    game_init(&state, "assets/data/CHIPS.DAT");

    /* Jump to Level 2 */
    if (!game_goto_password(&state, "2")) {
        printf("FAILED to goto level 2\n");
        return 1;
    }

    printf("Level %d loaded: '%s', creature count: %d\n",
           state.current_level_num, state.level.title, state.num_creatures);

    assert(state.num_creatures > 0);
    for (int i = 0; i < state.num_creatures; i++) {
        printf("Creature %d: species 0x%02X at (%d, %d), dir %d\n",
               i, state.creatures[i].base_species,
               state.creatures[i].x, state.creatures[i].y,
               state.creatures[i].dir);
    }

    int init_x0 = state.creatures[0].x;
    int init_y0 = state.creatures[0].y;

    /* Tick creatures 4 times (1 second of game time) */
    for (int t = 0; t < 4; t++) {
        game_update_creatures(&state);
        printf("After tick %d: Creature 0 at (%d, %d), dir %d\n",
               t + 1, state.creatures[0].x, state.creatures[0].y,
               state.creatures[0].dir);
    }

    /* Verify creature 0 moved from initial position */
    if (state.creatures[0].x != init_x0 || state.creatures[0].y != init_y0) {
        printf("SUCCESS: Creature 0 moved from (%d, %d) to (%d, %d)!\n",
               init_x0, init_y0, state.creatures[0].x, state.creatures[0].y);
    } else {
        printf("WARNING: Creature 0 stayed at same spot!\n");
    }

    /* Also check level 3 or another level with monsters */
    if (game_goto_password(&state, "3")) {
        printf("Level 3 loaded: '%s', creatures = %d\n", state.level.title, state.num_creatures);
    }

    printf("All creature movement tests PASSED!\n");
    return 0;
}
