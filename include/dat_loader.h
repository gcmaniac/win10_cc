#ifndef DAT_LOADER_H
#define DAT_LOADER_H

#include <stdbool.h>
#include "chips_types.h"

bool dat_init(const char *dat_path);
int  dat_get_level_count(void);
bool dat_load_level(int level_num, LevelData *out_level);
void dat_close(void);

#endif /* DAT_LOADER_H */
