#include "dat_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *s_fp = NULL;
static int s_total_levels = 0;
static long *s_level_offsets = NULL;

static void decompress_rle(const uint8_t *src, int src_len, uint8_t dst[GRID_H][GRID_W]) {
    int out_idx = 0;
    int in_idx = 0;
    while (in_idx < src_len && out_idx < GRID_W * GRID_H) {
        uint8_t b = src[in_idx++];
        if (b == 0xFF && in_idx + 1 < src_len) {
            uint8_t count = src[in_idx++];
            uint8_t val = src[in_idx++];
            for (int k = 0; k < count && out_idx < GRID_W * GRID_H; ++k) {
                int y = out_idx / GRID_W;
                int x = out_idx % GRID_W;
                dst[y][x] = val;
                out_idx++;
            }
        } else {
            int y = out_idx / GRID_W;
            int x = out_idx % GRID_W;
            dst[y][x] = b;
            out_idx++;
        }
    }
}

bool dat_init(const char *dat_path) {
    if (s_fp) {
        fclose(s_fp);
        s_fp = NULL;
    }
    if (s_level_offsets) {
        free(s_level_offsets);
        s_level_offsets = NULL;
    }

    s_fp = fopen(dat_path, "rb");
    if (!s_fp) {
        return false;
    }

    uint32_t magic = 0;
    uint16_t num_levels = 0;
    if (fread(&magic, 4, 1, s_fp) != 1 || fread(&num_levels, 2, 1, s_fp) != 1) {
        fclose(s_fp);
        s_fp = NULL;
        return false;
    }

    if (magic != 0x0002AAAC && magic != 0x0102AAAC) {
        fclose(s_fp);
        s_fp = NULL;
        return false;
    }

    s_total_levels = num_levels;
    s_level_offsets = (long *)malloc((s_total_levels + 1) * sizeof(long));
    if (!s_level_offsets) {
        fclose(s_fp);
        s_fp = NULL;
        return false;
    }

    /* Index level offsets */
    for (int i = 1; i <= s_total_levels; ++i) {
        s_level_offsets[i] = ftell(s_fp);
        uint16_t lvl_bytes = 0;
        if (fread(&lvl_bytes, 2, 1, s_fp) != 1) {
            break;
        }
        fseek(s_fp, s_level_offsets[i] + 2 + lvl_bytes, SEEK_SET);
    }

    return true;
}

int dat_get_level_count(void) {
    return s_total_levels;
}

bool dat_load_level(int level_num, LevelData *out_level) {
    if (!s_fp || !s_level_offsets || level_num < 1 || level_num > s_total_levels) {
        return false;
    }

    long offset = s_level_offsets[level_num];
    if (fseek(s_fp, offset, SEEK_SET) != 0) {
        return false;
    }

    memset(out_level, 0, sizeof(LevelData));

    uint16_t lvl_bytes = 0;
    uint16_t lvl_num = 0;
    uint16_t time_limit = 0;
    uint16_t chips_needed = 0;
    uint16_t map_detail = 0;

    fread(&lvl_bytes, 2, 1, s_fp);
    fread(&lvl_num, 2, 1, s_fp);
    fread(&time_limit, 2, 1, s_fp);
    fread(&chips_needed, 2, 1, s_fp);
    fread(&map_detail, 2, 1, s_fp);

    out_level->number = lvl_num;
    out_level->time_limit = time_limit;
    out_level->chips_needed = chips_needed;
    snprintf(out_level->title, sizeof(out_level->title), "LEVEL %d", lvl_num);
    snprintf(out_level->password, sizeof(out_level->password), "----");

    /* Layer 1 */
    uint16_t l1_sz = 0;
    fread(&l1_sz, 2, 1, s_fp);
    uint8_t *l1_buf = (uint8_t *)malloc(l1_sz);
    fread(l1_buf, 1, l1_sz, s_fp);
    decompress_rle(l1_buf, l1_sz, out_level->top_layer);
    free(l1_buf);

    /* Layer 2 */
    uint16_t l2_sz = 0;
    fread(&l2_sz, 2, 1, s_fp);
    uint8_t *l2_buf = (uint8_t *)malloc(l2_sz);
    fread(l2_buf, 1, l2_sz, s_fp);
    decompress_rle(l2_buf, l2_sz, out_level->bottom_layer);
    free(l2_buf);

    /* Optional fields */
    uint16_t opt_sz = 0;
    if (fread(&opt_sz, 2, 1, s_fp) == 1 && opt_sz > 0) {
        uint8_t *opt_buf = (uint8_t *)malloc(opt_sz);
        fread(opt_buf, 1, opt_sz, s_fp);
        int idx = 0;
        while (idx + 1 < opt_sz) {
            uint8_t f_type = opt_buf[idx++];
            uint8_t f_len  = opt_buf[idx++];
            if (idx + f_len > opt_sz) break;
            
            if (f_type == 3) { /* Title */
                int copy_len = f_len < (int)sizeof(out_level->title) - 1 ? f_len : (int)sizeof(out_level->title) - 1;
                memcpy(out_level->title, &opt_buf[idx], copy_len);
                out_level->title[copy_len] = '\0';
            } else if (f_type == 6) { /* Password XOR 0x99 */
                for (int k = 0; k < 4 && k < f_len; ++k) {
                    out_level->password[k] = (char)(opt_buf[idx + k] ^ 0x99);
                }
                out_level->password[4] = '\0';
            } else if (f_type == 7) { /* Hint */
                int copy_len = f_len < (int)sizeof(out_level->hint) - 1 ? f_len : (int)sizeof(out_level->hint) - 1;
                memcpy(out_level->hint, &opt_buf[idx], copy_len);
                out_level->hint[copy_len] = '\0';
            } else if (f_type == 10) { /* Creature moving order list */
                out_level->num_creature_coords = 0;
                for (int k = 0; k + 1 < f_len && out_level->num_creature_coords < MAX_CREATURES; k += 2) {
                    out_level->creature_coords[out_level->num_creature_coords].x = opt_buf[idx + k];
                    out_level->creature_coords[out_level->num_creature_coords].y = opt_buf[idx + k + 1];
                    out_level->num_creature_coords++;
                }
            }
            idx += f_len;
        }
        free(opt_buf);
    }

    /* Locate Chip and save initial copies */
    out_level->start_x = 0;
    out_level->start_y = 0;
    out_level->start_dir = DIR_SOUTH;

    for (int y = 0; y < GRID_H; ++y) {
        for (int x = 0; x < GRID_W; ++x) {
            uint8_t t = out_level->top_layer[y][x];
            if (t >= FC_CHIP_N && t <= FC_CHIP_E) {
                out_level->start_x = x;
                out_level->start_y = y;
                if (t == FC_CHIP_N) out_level->start_dir = DIR_NORTH;
                else if (t == FC_CHIP_W) out_level->start_dir = DIR_WEST;
                else if (t == FC_CHIP_S) out_level->start_dir = DIR_SOUTH;
                else if (t == FC_CHIP_E) out_level->start_dir = DIR_EAST;
            }
            out_level->initial_top[y][x] = out_level->top_layer[y][x];
            out_level->initial_bottom[y][x] = out_level->bottom_layer[y][x];
        }
    }

    return true;
}

void dat_close(void) {
    if (s_fp) {
        fclose(s_fp);
        s_fp = NULL;
    }
    if (s_level_offsets) {
        free(s_level_offsets);
        s_level_offsets = NULL;
    }
    s_total_levels = 0;
}
