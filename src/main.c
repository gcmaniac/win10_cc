#include <windows.h>
#include <stdio.h>
#include "chips_types.h"
#include "dat_loader.h"
#include "renderer.h"
#include "audio.h"
#include "game_logic.h"

#define TIMER_SECOND    1
#define TIMER_SLIDE     2
#define TIMER_CREATURE  3

/* Menu Command IDs */
#define IDM_GAME_RESTART   101
#define IDM_GAME_PASSWORD  102
#define IDM_GAME_NEXT      103
#define IDM_GAME_PREV      104
#define IDM_GAME_SOUND     105
#define IDM_GAME_EXIT      106
#define IDM_HELP_HINT      201
#define IDM_HELP_ABOUT     202

static GameState g_state;
static HWND      g_hwnd = NULL;

/* Password Dialog State */
static HWND s_hEditPass = NULL;
static bool s_passDlgOk = false;
static char s_enteredPass[32] = "";

static LRESULT CALLBACK PasswordDlgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            HFONT hFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);

            HWND hLabel = CreateWindowA("STATIC",
                "Enter 4-letter level password (e.g. BDHP, JXMJ)\nor level number (1 - 149):",
                WS_CHILD | WS_VISIBLE | SS_CENTER,
                15, 15, 300, 36, hwnd, NULL, NULL, NULL);
            SendMessageA(hLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

            s_hEditPass = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", "",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_UPPERCASE | ES_CENTER,
                70, 58, 190, 26, hwnd, (HMENU)1001, NULL, NULL);
            SendMessageA(s_hEditPass, WM_SETFONT, (WPARAM)hFont, TRUE);
            SendMessageA(s_hEditPass, EM_LIMITTEXT, 16, 0);

            HWND hBtnOk = CreateWindowA("BUTTON", "OK",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                65, 96, 85, 28, hwnd, (HMENU)IDOK, NULL, NULL);
            SendMessageA(hBtnOk, WM_SETFONT, (WPARAM)hFont, TRUE);

            HWND hBtnCancel = CreateWindowA("BUTTON", "Cancel",
                WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                180, 96, 85, 28, hwnd, (HMENU)IDCANCEL, NULL, NULL);
            SendMessageA(hBtnCancel, WM_SETFONT, (WPARAM)hFont, TRUE);

            SetFocus(s_hEditPass);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            if (wmId == IDOK) {
                GetWindowTextA(s_hEditPass, s_enteredPass, sizeof(s_enteredPass));
                s_passDlgOk = true;
                DestroyWindow(hwnd);
                return 0;
            } else if (wmId == IDCANCEL) {
                s_passDlgOk = false;
                DestroyWindow(hwnd);
                return 0;
            }
            break;
        }

        case WM_CLOSE: {
            s_passDlgOk = false;
            DestroyWindow(hwnd);
            return 0;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
    return 0;
}

static void ShowPasswordDialog(HWND hwndParent) {
    s_passDlgOk = false;
    s_enteredPass[0] = '\0';

    WNDCLASSEXA wc = {0};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = PasswordDlgProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = "ChipsPassDlgClass";
    wc.hIcon = LoadIconA(GetModuleHandle(NULL), MAKEINTRESOURCEA(1));
    wc.hIconSm = (HICON)LoadImageA(GetModuleHandle(NULL), MAKEINTRESOURCEA(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    RegisterClassExA(&wc);

    RECT parentRect;
    GetWindowRect(hwndParent, &parentRect);
    int dlgW = 345;
    int dlgH = 175;
    int dlgX = parentRect.left + (parentRect.right - parentRect.left - dlgW) / 2;
    int dlgY = parentRect.top + (parentRect.bottom - parentRect.top - dlgH) / 2;

    HWND hDlg = CreateWindowExA(
        WS_EX_DLGMODALFRAME | WS_EX_TOPMOST,
        "ChipsPassDlgClass",
        "Go to Level / Password",
        WS_POPUP | WS_CAPTION | WS_SYSMENU | WS_VISIBLE,
        dlgX, dlgY, dlgW, dlgH,
        hwndParent, NULL, GetModuleHandle(NULL), NULL
    );

    EnableWindow(hwndParent, FALSE);

    MSG msg;
    while (IsWindow(hDlg) && GetMessageA(&msg, NULL, 0, 0)) {
        if (msg.message == WM_KEYDOWN) {
            if (msg.wParam == VK_RETURN) {
                SendMessageA(hDlg, WM_COMMAND, IDOK, 0);
                continue;
            } else if (msg.wParam == VK_ESCAPE) {
                SendMessageA(hDlg, WM_COMMAND, IDCANCEL, 0);
                continue;
            }
        }
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    EnableWindow(hwndParent, TRUE);
    SetForegroundWindow(hwndParent);
    SetFocus(hwndParent);

    if (s_passDlgOk && strlen(s_enteredPass) > 0) {
        if (!game_goto_password(&g_state, s_enteredPass)) {
            char failMsg[128];
            snprintf(failMsg, sizeof(failMsg), "Invalid password or level number '%s'!", s_enteredPass);
            MessageBoxA(hwndParent, failMsg, "Invalid Password", MB_ICONWARNING);
        }
        InvalidateRect(hwndParent, NULL, FALSE);
    }
}

static HMENU CreateGameMenu(void) {
    HMENU hMenuBar = CreateMenu();
    HMENU hGameMenu = CreatePopupMenu();
    HMENU hHelpMenu = CreatePopupMenu();

    AppendMenuA(hGameMenu, MF_STRING, IDM_GAME_RESTART, "Restart Level\tF2 / R");
    AppendMenuA(hGameMenu, MF_STRING, IDM_GAME_PASSWORD, "Go to Level / Password...\tCtrl+G / F3");
    AppendMenuA(hGameMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hGameMenu, MF_STRING, IDM_GAME_NEXT, "Next Level\tN / PgDn");
    AppendMenuA(hGameMenu, MF_STRING, IDM_GAME_PREV, "Previous Level\tP / PgUp");
    AppendMenuA(hGameMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hGameMenu, MF_STRING, IDM_GAME_SOUND, "Toggle Sound & Music\tM");
    AppendMenuA(hGameMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hGameMenu, MF_STRING, IDM_GAME_EXIT, "Exit\tEsc");

    AppendMenuA(hHelpMenu, MF_STRING, IDM_HELP_HINT, "Show Level Hint\tH");
    AppendMenuA(hHelpMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuA(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, "About Chip's Challenge...");

    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hGameMenu, "&Game");
    AppendMenuA(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, "&Help");

    return hMenuBar;
}

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_CREATE: {
            g_hwnd = hwnd;
            audio_init("assets/audio");
            if (!renderer_init(hwnd, "assets/sprites")) {
                MessageBoxA(hwnd, "Failed to load sprite tiles from assets/sprites!\nPlease run setup_requirements.bat first.", "Error", MB_ICONERROR);
                PostQuitMessage(1);
                return 0;
            }
            game_init(&g_state, "assets/data/CHIPS.DAT");
            SetTimer(hwnd, TIMER_SECOND, 1000, NULL);
            SetTimer(hwnd, TIMER_SLIDE, 180, NULL);
            SetTimer(hwnd, TIMER_CREATURE, 250, NULL);
            return 0;
        }

        case WM_COMMAND: {
            int wmId = LOWORD(wParam);
            switch (wmId) {
                case IDM_GAME_RESTART:
                    game_restart_level(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case IDM_GAME_PASSWORD:
                    ShowPasswordDialog(hwnd);
                    break;
                case IDM_GAME_NEXT:
                    game_next_level(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case IDM_GAME_PREV:
                    game_prev_level(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case IDM_GAME_SOUND:
                    audio_toggle_sound();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case IDM_GAME_EXIT:
                    DestroyWindow(hwnd);
                    break;
                case IDM_HELP_HINT:
                    g_state.show_hint = !g_state.show_hint;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case IDM_HELP_ABOUT:
                    MessageBoxA(hwnd,
                        "Chip's Challenge - Windows 10 Native Edition (64-bit)\n"
                        "Engine reimplemented natively for modern 64-bit Windows.\n\n"
                        "Controls:\n"
                        "- Movement: Arrow Keys / WASD\n"
                        "- Level Password: Game -> Go to Level (Ctrl+G / F3)\n"
                        "- Restart: F2 / R\n"
                        "- Sound: M\n"
                        "- Hint: H",
                        "About Chip's Challenge", MB_ICONINFORMATION);
                    break;
            }
            return 0;
        }

        case WM_TIMER: {
            if (wParam == TIMER_SECOND) {
                game_update_timer(&g_state);
                InvalidateRect(hwnd, NULL, FALSE);
            } else if (wParam == TIMER_SLIDE) {
                if (g_state.is_sliding) {
                    game_update_sliding(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            } else if (wParam == TIMER_CREATURE) {
                if (g_state.chip_alive && !g_state.level_completed) {
                    game_update_creatures(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            return 0;
        }

        case WM_KEYDOWN: {
            /* Check Ctrl+G for Password */
            if (wParam == 'G') {
                if ((GetKeyState(VK_CONTROL) & 0x8000) != 0) {
                    ShowPasswordDialog(hwnd);
                    return 0;
                }
            }

            switch (wParam) {
                case VK_UP:
                case 'W':
                    game_move_player(&g_state, DIR_NORTH);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case VK_LEFT:
                case 'A':
                    game_move_player(&g_state, DIR_WEST);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case VK_DOWN:
                case 'S':
                    game_move_player(&g_state, DIR_SOUTH);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                case VK_RIGHT:
                case 'D':
                    game_move_player(&g_state, DIR_EAST);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;

                case 'R':
                case VK_F2:
                    game_restart_level(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;

                case VK_F3:
                    ShowPasswordDialog(hwnd);
                    break;

                case 'N':
                case VK_NEXT:
                    game_next_level(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;

                case 'P':
                case VK_PRIOR:
                    game_prev_level(&g_state);
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;

                case 'M':
                    audio_toggle_sound();
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;

                case 'H':
                    g_state.show_hint = !g_state.show_hint;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;

                case VK_SPACE:
                case VK_RETURN:
                    if (g_state.level_completed) {
                        game_next_level(&g_state);
                        InvalidateRect(hwnd, NULL, FALSE);
                    }
                    break;

                case VK_ESCAPE:
                    DestroyWindow(hwnd);
                    break;
            }
            return 0;
        }

        case WM_PAINT: {
            renderer_render(hwnd, &g_state);
            return 0;
        }

        case WM_ERASEBKGND:
            return 1; /* Handled in double buffer */

        case WM_DESTROY: {
            KillTimer(hwnd, TIMER_SECOND);
            KillTimer(hwnd, TIMER_SLIDE);
            KillTimer(hwnd, TIMER_CREATURE);
            renderer_cleanup();
            audio_cleanup();
            dat_close();
            PostQuitMessage(0);
            return 0;
        }

        default:
            return DefWindowProcA(hwnd, msg, wParam, lParam);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;

    const char *className = "ChipsChallengeWin10Class";

    WNDCLASSEXA wc = {0};
    wc.cbSize        = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc   = WndProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = className;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    HICON hAppIcon   = LoadIconA(hInstance, MAKEINTRESOURCEA(1));
    if (!hAppIcon) hAppIcon = LoadIconA(NULL, IDI_APPLICATION);
    HICON hAppIconSm = (HICON)LoadImageA(hInstance, MAKEINTRESOURCEA(1), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

    wc.hIcon         = hAppIcon;
    wc.hIconSm       = hAppIconSm;
    wc.hbrBackground = NULL;

    if (!RegisterClassExA(&wc)) {
        MessageBoxA(NULL, "Failed to register window class!", "Error", MB_ICONERROR);
        return 1;
    }

    int clientW = 860;
    int clientH = 620;

    HMENU hMenu = CreateGameMenu();

    RECT wr = { 0, 0, clientW, clientH };
    AdjustWindowRect(&wr, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, TRUE);
    int windowW = wr.right - wr.left;
    int windowH = wr.bottom - wr.top;

    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int posX = (screenW - windowW) / 2;
    int posY = (screenH - windowH) / 2;

    HWND hwnd = CreateWindowExA(
        0,
        className,
        "Chip's Challenge - Windows 10 Native Edition",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        posX, posY, windowW, windowH,
        NULL, hMenu, hInstance, NULL
    );

    if (!hwnd) {
        MessageBoxA(NULL, "Failed to create main window!", "Error", MB_ICONERROR);
        return 1;
    }

    if (hAppIcon) SendMessageA(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hAppIcon);
    if (hAppIconSm) SendMessageA(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hAppIconSm);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    return (int)msg.wParam;
}
