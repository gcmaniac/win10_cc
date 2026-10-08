#ifndef CHIPS_RENDERER_H
#define CHIPS_RENDERER_H

#include <windows.h>
#include <stdbool.h>
#include "chips_types.h"

#define SCALE_FACTOR 2  /* 2x scaling: 32px tile -> 64px, 288x288 viewport -> 576x576 */

bool renderer_init(HWND hwnd, const char *sprites_dir);
void renderer_render(HWND hwnd, const GameState *state);
void renderer_cleanup(void);

#endif /* CHIPS_RENDERER_H */
