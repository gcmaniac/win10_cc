#include "renderer.h"
#include <stdio.h>

static HBITMAP s_hbm_tiles = NULL;
static HDC     s_hdc_tiles = NULL;
static HFONT   s_hfont_ui = NULL;
static HFONT   s_hfont_title = NULL;
static HFONT   s_hfont_mono = NULL;

typedef struct TileCoord {
    int col;
    int row;
    bool transparent;
} TileCoord;

static TileCoord get_tile_info(uint8_t code) {
    TileCoord tc = {0, 0, false}; /* default empty floor */
    switch (code) {
        case FC_EMPTY:            tc.col = 0; tc.row = 0;  tc.transparent = false; break;
        case FC_WALL:             tc.col = 0; tc.row = 1;  tc.transparent = false; break;
        case FC_CHIP:             tc.col = 0; tc.row = 2;  tc.transparent = false; break;
        case FC_WATER:            tc.col = 0; tc.row = 3;  tc.transparent = false; break;
        case FC_FIRE:             tc.col = 0; tc.row = 4;  tc.transparent = false; break;
        case FC_HIDDEN_WALL_PERM: tc.col = 0; tc.row = 0;  tc.transparent = false; break;
        case FC_WALL_NORTH:       tc.col = 0; tc.row = 6;  tc.transparent = false; break;
        case FC_WALL_WEST:        tc.col = 0; tc.row = 7;  tc.transparent = false; break;
        case FC_WALL_SOUTH:       tc.col = 0; tc.row = 8;  tc.transparent = false; break;
        case FC_WALL_EAST:        tc.col = 0; tc.row = 9;  tc.transparent = false; break;
        case FC_BLOCK_STATIC:     tc.col = 0; tc.row = 10; tc.transparent = false; break;
        case FC_DIRT:             tc.col = 0; tc.row = 11; tc.transparent = false; break;
        case FC_ICE:              tc.col = 0; tc.row = 12; tc.transparent = false; break;
        case FC_SLIDE_SOUTH:      tc.col = 0; tc.row = 13; tc.transparent = false; break;
        case FC_CLONE_BLOCK_N:    tc.col = 0; tc.row = 14; tc.transparent = false; break;
        case FC_CLONE_BLOCK_W:    tc.col = 0; tc.row = 15; tc.transparent = false; break;
        case FC_CLONE_BLOCK_S:    tc.col = 1; tc.row = 0;  tc.transparent = false; break;
        case FC_CLONE_BLOCK_E:    tc.col = 1; tc.row = 1;  tc.transparent = false; break;
        case FC_SLIDE_NORTH:      tc.col = 1; tc.row = 2;  tc.transparent = false; break;
        case FC_SLIDE_EAST:       tc.col = 1; tc.row = 3;  tc.transparent = false; break;
        case FC_SLIDE_WEST:       tc.col = 1; tc.row = 4;  tc.transparent = false; break;
        case FC_EXIT:             tc.col = 1; tc.row = 5;  tc.transparent = false; break;
        case FC_DOOR_BLUE:        tc.col = 1; tc.row = 6;  tc.transparent = false; break;
        case FC_DOOR_RED:         tc.col = 1; tc.row = 7;  tc.transparent = false; break;
        case FC_DOOR_GREEN:       tc.col = 1; tc.row = 8;  tc.transparent = false; break;
        case FC_DOOR_YELLOW:      tc.col = 1; tc.row = 9;  tc.transparent = false; break;
        case FC_ICE_SE:           tc.col = 1; tc.row = 10; tc.transparent = false; break;
        case FC_ICE_SW:           tc.col = 1; tc.row = 11; tc.transparent = false; break;
        case FC_ICE_NW:           tc.col = 1; tc.row = 12; tc.transparent = false; break;
        case FC_ICE_NE:           tc.col = 1; tc.row = 13; tc.transparent = false; break;
        case FC_BLUE_WALL_REAL:   tc.col = 1; tc.row = 14; tc.transparent = false; break;
        case FC_BLUE_WALL_FAKE:   tc.col = 1; tc.row = 15; tc.transparent = false; break;
        case FC_BURGLAR:          tc.col = 2; tc.row = 1;  tc.transparent = false; break;
        case FC_SOCKET:           tc.col = 2; tc.row = 2;  tc.transparent = false; break;
        case FC_BUTTON_GREEN:     tc.col = 2; tc.row = 3;  tc.transparent = false; break;
        case FC_BUTTON_RED:       tc.col = 2; tc.row = 4;  tc.transparent = false; break;
        case FC_SWITCH_CLOSED:    tc.col = 2; tc.row = 5;  tc.transparent = false; break;
        case FC_SWITCH_OPEN:      tc.col = 2; tc.row = 6;  tc.transparent = false; break;
        case FC_BUTTON_BROWN:     tc.col = 2; tc.row = 7;  tc.transparent = false; break;
        case FC_BUTTON_BLUE:      tc.col = 2; tc.row = 8;  tc.transparent = false; break;
        case FC_TELEPORT:         tc.col = 2; tc.row = 9;  tc.transparent = false; break;
        case FC_BOMB:             tc.col = 2; tc.row = 10; tc.transparent = false; break;
        case FC_TRAP:             tc.col = 2; tc.row = 11; tc.transparent = false; break;
        case FC_HIDDEN_WALL_TEMP: tc.col = 2; tc.row = 12; tc.transparent = false; break;
        case FC_GRAVEL:           tc.col = 2; tc.row = 13; tc.transparent = false; break;
        case FC_POPUP_WALL:       tc.col = 2; tc.row = 14; tc.transparent = false; break;
        case FC_HINT:             tc.col = 2; tc.row = 15; tc.transparent = false; break;
        case FC_WALL_SE:          tc.col = 3; tc.row = 0;  tc.transparent = false; break;
        case FC_CLONE_MACHINE:    tc.col = 3; tc.row = 1;  tc.transparent = false; break;
        case FC_SLIDE_RANDOM:     tc.col = 3; tc.row = 2;  tc.transparent = false; break;
        case FC_DROWNED_CHIP:     tc.col = 3; tc.row = 3;  tc.transparent = false; break;
        case FC_BURNED_CHIP:      tc.col = 3; tc.row = 4;  tc.transparent = false; break;
        case FC_BOMBED_CHIP:      tc.col = 3; tc.row = 5;  tc.transparent = false; break;
        case FC_EXITED_CHIP:      tc.col = 3; tc.row = 9;  tc.transparent = false; break;
        
        case FC_KEY_BLUE:         tc.col = 6; tc.row = 4;  tc.transparent = true;  break;
        case FC_KEY_RED:          tc.col = 6; tc.row = 5;  tc.transparent = true;  break;
        case FC_KEY_GREEN:        tc.col = 6; tc.row = 6;  tc.transparent = true;  break;
        case FC_KEY_YELLOW:       tc.col = 6; tc.row = 7;  tc.transparent = true;  break;
        case FC_BOOTS_WATER:      tc.col = 6; tc.row = 8;  tc.transparent = true;  break;
        case FC_BOOTS_FIRE:       tc.col = 6; tc.row = 9;  tc.transparent = true;  break;
        case FC_BOOTS_ICE:        tc.col = 6; tc.row = 10; tc.transparent = true;  break;
        case FC_BOOTS_SLIDE:      tc.col = 6; tc.row = 11; tc.transparent = true;  break;
        
        case FC_CHIP_N:           tc.col = 6; tc.row = 12; tc.transparent = true;  break;
        case FC_CHIP_W:           tc.col = 6; tc.row = 13; tc.transparent = true;  break;
        case FC_CHIP_S:           tc.col = 6; tc.row = 14; tc.transparent = true;  break;
        case FC_CHIP_E:           tc.col = 6; tc.row = 15; tc.transparent = true;  break;
        
        case FC_BUG_N:            tc.col = 4; tc.row = 0;  tc.transparent = true;  break;
        case FC_BUG_W:            tc.col = 4; tc.row = 1;  tc.transparent = true;  break;
        case FC_BUG_S:            tc.col = 4; tc.row = 2;  tc.transparent = true;  break;
        case FC_BUG_E:            tc.col = 4; tc.row = 3;  tc.transparent = true;  break;
        case FC_FIREBALL_N:       tc.col = 4; tc.row = 4;  tc.transparent = true;  break;
        case FC_FIREBALL_W:       tc.col = 4; tc.row = 5;  tc.transparent = true;  break;
        case FC_FIREBALL_S:       tc.col = 4; tc.row = 6;  tc.transparent = true;  break;
        case FC_FIREBALL_E:       tc.col = 4; tc.row = 7;  tc.transparent = true;  break;
        case FC_BALL_N:           tc.col = 4; tc.row = 8;  tc.transparent = true;  break;
        case FC_BALL_W:           tc.col = 4; tc.row = 9;  tc.transparent = true;  break;
        case FC_BALL_S:           tc.col = 4; tc.row = 10; tc.transparent = true;  break;
        case FC_BALL_E:           tc.col = 4; tc.row = 11; tc.transparent = true;  break;
        case FC_TANK_N:           tc.col = 4; tc.row = 12; tc.transparent = true;  break;
        case FC_TANK_W:           tc.col = 4; tc.row = 13; tc.transparent = true;  break;
        case FC_TANK_S:           tc.col = 4; tc.row = 14; tc.transparent = true;  break;
        case FC_TANK_E:           tc.col = 4; tc.row = 15; tc.transparent = true;  break;
        case FC_GLIDER_N:         tc.col = 5; tc.row = 0;  tc.transparent = true;  break;
        case FC_GLIDER_W:         tc.col = 5; tc.row = 1;  tc.transparent = true;  break;
        case FC_GLIDER_S:         tc.col = 5; tc.row = 2;  tc.transparent = true;  break;
        case FC_GLIDER_E:         tc.col = 5; tc.row = 3;  tc.transparent = true;  break;
        case FC_TEETH_N:          tc.col = 5; tc.row = 4;  tc.transparent = true;  break;
        case FC_TEETH_W:          tc.col = 5; tc.row = 5;  tc.transparent = true;  break;
        case FC_TEETH_S:          tc.col = 5; tc.row = 6;  tc.transparent = true;  break;
        case FC_TEETH_E:          tc.col = 5; tc.row = 7;  tc.transparent = true;  break;
        case FC_WALKER_N:         tc.col = 5; tc.row = 8;  tc.transparent = true;  break;
        case FC_WALKER_W:         tc.col = 5; tc.row = 9;  tc.transparent = true;  break;
        case FC_WALKER_S:         tc.col = 5; tc.row = 10; tc.transparent = true;  break;
        case FC_WALKER_E:         tc.col = 5; tc.row = 11; tc.transparent = true;  break;
        case FC_PARAMECIUM_N:     tc.col = 6; tc.row = 0;  tc.transparent = true;  break;
        case FC_PARAMECIUM_W:     tc.col = 6; tc.row = 1;  tc.transparent = true;  break;
        case FC_PARAMECIUM_S:     tc.col = 6; tc.row = 2;  tc.transparent = true;  break;
        case FC_PARAMECIUM_E:     tc.col = 6; tc.row = 3;  tc.transparent = true;  break;

        default:                  tc.col = 0; tc.row = 0;  tc.transparent = false; break;
    }
    return tc;
}

bool renderer_init(HWND hwnd, const char *sprites_dir) {
    char bmp_path[MAX_PATH];
    snprintf(bmp_path, sizeof(bmp_path), "%s/tiles_color_rgb.bmp", sprites_dir);

    s_hbm_tiles = (HBITMAP)LoadImageA(NULL, bmp_path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (!s_hbm_tiles) {
        snprintf(bmp_path, sizeof(bmp_path), "%s/tiles_color.bmp", sprites_dir);
        s_hbm_tiles = (HBITMAP)LoadImageA(NULL, bmp_path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    }
    if (!s_hbm_tiles) {
        return false;
    }

    HDC hdc = GetDC(hwnd);
    s_hdc_tiles = CreateCompatibleDC(hdc);
    SelectObject(s_hdc_tiles, s_hbm_tiles);
    ReleaseDC(hwnd, hdc);

    s_hfont_ui = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                             DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                             CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    s_hfont_title = CreateFontA(20, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_SWISS, "Segoe UI");
    s_hfont_mono = CreateFontA(16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
                              ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                              CLEARTYPE_QUALITY, FIXED_PITCH | FF_MODERN, "Consolas");

    return true;
}

static void draw_tile(HDC hdcDst, int dstX, int dstY, int dstW, int dstH, uint8_t code) {
    TileCoord tc = get_tile_info(code);
    int srcX = tc.col * 32;
    int srcY = tc.row * 32;

    if (tc.transparent) {
        TransparentBlt(hdcDst, dstX, dstY, dstW, dstH,
                       s_hdc_tiles, srcX, srcY, 32, 32,
                       RGB(255, 255, 255));
    } else {
        StretchBlt(hdcDst, dstX, dstY, dstW, dstH,
                   s_hdc_tiles, srcX, srcY, 32, 32,
                   SRCCOPY);
    }
}

void renderer_render(HWND hwnd, const GameState *state) {
    PAINTSTRUCT ps;
    HDC hdcWindow = BeginPaint(hwnd, &ps);

    RECT clientRect;
    GetClientRect(hwnd, &clientRect);
    int clientW = clientRect.right - clientRect.left;
    int clientH = clientRect.bottom - clientRect.top;

    HDC hdcMem = CreateCompatibleDC(hdcWindow);
    HBITMAP hbmMem = CreateCompatibleBitmap(hdcWindow, clientW, clientH);
    HGDIOBJ oldBm = SelectObject(hdcMem, hbmMem);

    /* Background: Sleek Dark Slate Slate */
    HBRUSH bgBrush = CreateSolidBrush(RGB(24, 28, 36));
    FillRect(hdcMem, &clientRect, bgBrush);
    DeleteObject(bgBrush);

    int scaled_tile = TILE_SIZE * SCALE_FACTOR; /* 64 px */
    int view_px = VIEW_TILES * scaled_tile;     /* 576 px */
    int view_x0 = 16;
    int view_y0 = 16;

    /* Viewport frame border */
    HBRUSH borderBrush = CreateSolidBrush(RGB(48, 54, 70));
    RECT borderRect = { view_x0 - 3, view_y0 - 3, view_x0 + view_px + 3, view_y0 + view_px + 3 };
    FillRect(hdcMem, &borderRect, borderBrush);
    DeleteObject(borderBrush);

    /* Draw 9x9 Viewport */
    int center_view = VIEW_TILES / 2; /* 4 */
    for (int vy = 0; vy < VIEW_TILES; ++vy) {
        for (int vx = 0; vx < VIEW_TILES; ++vx) {
            int map_x = state->chip_x - center_view + vx;
            int map_y = state->chip_y - center_view + vy;

            int dstX = view_x0 + vx * scaled_tile;
            int dstY = view_y0 + vy * scaled_tile;

            if (map_x < 0 || map_x >= GRID_W || map_y < 0 || map_y >= GRID_H) {
                /* Outside boundary */
                HBRUSH voidBrush = CreateSolidBrush(RGB(12, 14, 18));
                RECT tileRect = { dstX, dstY, dstX + scaled_tile, dstY + scaled_tile };
                FillRect(hdcMem, &tileRect, voidBrush);
                DeleteObject(voidBrush);
            } else {
                uint8_t floor_code = state->level.bottom_layer[map_y][map_x];
                uint8_t top_code   = state->level.top_layer[map_y][map_x];

                /* Floor / bottom layer */
                draw_tile(hdcMem, dstX, dstY, scaled_tile, scaled_tile, floor_code);

                /* Top layer */
                if (top_code != FC_EMPTY) {
                    draw_tile(hdcMem, dstX, dstY, scaled_tile, scaled_tile, top_code);
                }
            }
        }
    }

    /* Draw Player Chip in center (tile 4, 4) */
    int chip_dstX = view_x0 + center_view * scaled_tile;
    int chip_dstY = view_y0 + center_view * scaled_tile;
    if (state->chip_alive) {
        uint8_t chip_sprite = FC_CHIP_S;
        if (state->chip_dir == DIR_NORTH) chip_sprite = FC_CHIP_N;
        else if (state->chip_dir == DIR_WEST) chip_sprite = FC_CHIP_W;
        else if (state->chip_dir == DIR_SOUTH) chip_sprite = FC_CHIP_S;
        else if (state->chip_dir == DIR_EAST) chip_sprite = FC_CHIP_E;
        draw_tile(hdcMem, chip_dstX, chip_dstY, scaled_tile, scaled_tile, chip_sprite);
    }

    /* Draw UI Panel on the Right */
    int ui_x = view_x0 + view_px + 24;
    int ui_y = view_y0;

    SetBkMode(hdcMem, TRANSPARENT);

    /* Level Title Header */
    SelectObject(hdcMem, s_hfont_title);
    SetTextColor(hdcMem, RGB(255, 215, 0)); /* Gold */
    char title_buf[128];
    snprintf(title_buf, sizeof(title_buf), "LEVEL %d of %d", state->current_level_num, state->total_levels);
    TextOutA(hdcMem, ui_x, ui_y, title_buf, (int)strlen(title_buf));

    SelectObject(hdcMem, s_hfont_ui);
    SetTextColor(hdcMem, RGB(230, 230, 230));
    TextOutA(hdcMem, ui_x, ui_y + 26, state->level.title, (int)strlen(state->level.title));

    /* Password */
    SelectObject(hdcMem, s_hfont_mono);
    SetTextColor(hdcMem, RGB(100, 200, 255));
    char pass_buf[64];
    snprintf(pass_buf, sizeof(pass_buf), "PASSWORD: %s", state->level.password);
    TextOutA(hdcMem, ui_x, ui_y + 54, pass_buf, (int)strlen(pass_buf));

    /* Separator */
    HPEN sepPen = CreatePen(PS_SOLID, 1, RGB(50, 60, 80));
    HGDIOBJ oldPen = SelectObject(hdcMem, sepPen);
    MoveToEx(hdcMem, ui_x, ui_y + 80, NULL);
    LineTo(hdcMem, ui_x + 220, ui_y + 80);
    SelectObject(hdcMem, oldPen);
    DeleteObject(sepPen);

    /* Counters Panel */
    SelectObject(hdcMem, s_hfont_ui);
    SetTextColor(hdcMem, RGB(255, 255, 255));

    char stat_buf[64];
    /* Chips Remaining */
    snprintf(stat_buf, sizeof(stat_buf), "CHIPS REMAINING: %d", state->chips_remaining);
    draw_tile(hdcMem, ui_x, ui_y + 95, 28, 28, FC_CHIP);
    TextOutA(hdcMem, ui_x + 36, ui_y + 100, stat_buf, (int)strlen(stat_buf));

    /* Time Remaining */
    if (state->level.time_limit > 0) {
        snprintf(stat_buf, sizeof(stat_buf), "TIME: %d s", state->time_remaining);
    } else {
        snprintf(stat_buf, sizeof(stat_buf), "TIME: NO LIMIT");
    }
    TextOutA(hdcMem, ui_x + 36, ui_y + 135, stat_buf, (int)strlen(stat_buf));

    /* Inventory Section */
    SetTextColor(hdcMem, RGB(180, 200, 240));
    TextOutA(hdcMem, ui_x, ui_y + 175, "INVENTORY KEYS:", 15);

    /* Keys */
    draw_tile(hdcMem, ui_x,       ui_y + 200, 28, 28, FC_KEY_BLUE);
    draw_tile(hdcMem, ui_x + 55,  ui_y + 200, 28, 28, FC_KEY_RED);
    draw_tile(hdcMem, ui_x + 110, ui_y + 200, 28, 28, FC_KEY_GREEN);
    draw_tile(hdcMem, ui_x + 165, ui_y + 200, 28, 28, FC_KEY_YELLOW);

    snprintf(stat_buf, sizeof(stat_buf), "x%d", state->inventory.keys_blue);
    TextOutA(hdcMem, ui_x + 30, ui_y + 206, stat_buf, (int)strlen(stat_buf));

    snprintf(stat_buf, sizeof(stat_buf), "x%d", state->inventory.keys_red);
    TextOutA(hdcMem, ui_x + 85, ui_y + 206, stat_buf, (int)strlen(stat_buf));

    snprintf(stat_buf, sizeof(stat_buf), "x%d", state->inventory.keys_green);
    TextOutA(hdcMem, ui_x + 140, ui_y + 206, stat_buf, (int)strlen(stat_buf));

    snprintf(stat_buf, sizeof(stat_buf), "x%d", state->inventory.keys_yellow);
    TextOutA(hdcMem, ui_x + 195, ui_y + 206, stat_buf, (int)strlen(stat_buf));

    /* Boots Section */
    TextOutA(hdcMem, ui_x, ui_y + 245, "SPECIAL BOOTS:", 14);
    if (state->inventory.has_flippers) draw_tile(hdcMem, ui_x, ui_y + 270, 28, 28, FC_BOOTS_WATER);
    if (state->inventory.has_fireboots) draw_tile(hdcMem, ui_x + 40, ui_y + 270, 28, 28, FC_BOOTS_FIRE);
    if (state->inventory.has_iceskates) draw_tile(hdcMem, ui_x + 80, ui_y + 270, 28, 28, FC_BOOTS_ICE);
    if (state->inventory.has_suctionboots) draw_tile(hdcMem, ui_x + 120, ui_y + 270, 28, 28, FC_BOOTS_SLIDE);

    /* Controls Help */
    SetTextColor(hdcMem, RGB(130, 145, 170));
    TextOutA(hdcMem, ui_x, ui_y + 320, "CONTROLS:", 9);
    TextOutA(hdcMem, ui_x, ui_y + 342, "Arrows / WASD: Move", 19);
    TextOutA(hdcMem, ui_x, ui_y + 364, "R: Restart Level", 16);
    TextOutA(hdcMem, ui_x, ui_y + 386, "N / P: Next / Prev Level", 24);
    TextOutA(hdcMem, ui_x, ui_y + 408, "M: Toggle Sound & Music", 23);
    TextOutA(hdcMem, ui_x, ui_y + 430, "H: Show Hint", 12);

    /* Hint text or status box */
    if (state->show_hint && strlen(state->level.hint) > 0) {
        RECT hintRect = { ui_x - 5, ui_y + 465, ui_x + 235, ui_y + 580 };
        HBRUSH hintBg = CreateSolidBrush(RGB(35, 45, 60));
        FillRect(hdcMem, &hintRect, hintBg);
        DeleteObject(hintBg);
        FrameRect(hdcMem, &hintRect, borderBrush);

        SetTextColor(hdcMem, RGB(255, 230, 150));
        RECT textRect = { ui_x + 5, ui_y + 472, ui_x + 225, ui_y + 575 };
        DrawTextA(hdcMem, state->level.hint, -1, &textRect, DT_WORDBREAK);
    } else if (strlen(state->status_message) > 0) {
        SetTextColor(hdcMem, RGB(255, 100, 100));
        TextOutA(hdcMem, ui_x, ui_y + 470, state->status_message, (int)strlen(state->status_message));
    }

    /* Overlay Message if Completed or Dead */
    if (state->level_completed) {
        RECT bannerRect = { view_x0 + 40, view_y0 + view_px / 2 - 40, view_x0 + view_px - 40, view_y0 + view_px / 2 + 40 };
        HBRUSH bannerBg = CreateSolidBrush(RGB(20, 80, 40));
        FillRect(hdcMem, &bannerRect, bannerBg);
        DeleteObject(bannerBg);

        SelectObject(hdcMem, s_hfont_title);
        SetTextColor(hdcMem, RGB(255, 255, 255));
        RECT textR1 = { bannerRect.left, bannerRect.top + 10, bannerRect.right, bannerRect.top + 35 };
        DrawTextA(hdcMem, "LEVEL COMPLETED!", -1, &textR1, DT_CENTER | DT_SINGLELINE);

        SelectObject(hdcMem, s_hfont_ui);
        SetTextColor(hdcMem, RGB(200, 255, 200));
        RECT textR2 = { bannerRect.left, bannerRect.top + 45, bannerRect.right, bannerRect.bottom };
        DrawTextA(hdcMem, "Press ENTER or SPACE for Next Level", -1, &textR2, DT_CENTER | DT_SINGLELINE);
    } else if (!state->chip_alive) {
        RECT bannerRect = { view_x0 + 40, view_y0 + view_px / 2 - 40, view_x0 + view_px - 40, view_y0 + view_px / 2 + 40 };
        HBRUSH bannerBg = CreateSolidBrush(RGB(100, 20, 20));
        FillRect(hdcMem, &bannerRect, bannerBg);
        DeleteObject(bannerBg);

        SelectObject(hdcMem, s_hfont_title);
        SetTextColor(hdcMem, RGB(255, 255, 255));
        RECT textR1 = { bannerRect.left, bannerRect.top + 10, bannerRect.right, bannerRect.top + 35 };
        DrawTextA(hdcMem, "OOPS! BUMMER!", -1, &textR1, DT_CENTER | DT_SINGLELINE);

        SelectObject(hdcMem, s_hfont_ui);
        SetTextColor(hdcMem, RGB(255, 200, 200));
        RECT textR2 = { bannerRect.left, bannerRect.top + 45, bannerRect.right, bannerRect.bottom };
        DrawTextA(hdcMem, "Press 'R' to Restart Level", -1, &textR2, DT_CENTER | DT_SINGLELINE);
    }

    /* Flip to screen */
    BitBlt(hdcWindow, 0, 0, clientW, clientH, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, oldBm);
    DeleteObject(hbmMem);
    DeleteDC(hdcMem);

    EndPaint(hwnd, &ps);
}

void renderer_cleanup(void) {
    if (s_hdc_tiles) {
        DeleteDC(s_hdc_tiles);
        s_hdc_tiles = NULL;
    }
    if (s_hbm_tiles) {
        DeleteObject(s_hbm_tiles);
        s_hbm_tiles = NULL;
    }
    if (s_hfont_ui) DeleteObject(s_hfont_ui);
    if (s_hfont_title) DeleteObject(s_hfont_title);
    if (s_hfont_mono) DeleteObject(s_hfont_mono);
}
