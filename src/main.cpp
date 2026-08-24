#include <windows.h>
#include <fstream>
#include <thread>
#include <atomic>
#include "sudoku_engine.h"
#include "ui_renderer.h"

#define TIMER_ID 1

// Global Application State
SamuraiSudoku g_game;
UIState g_ui;
std::atomic<bool> g_generating(false);

const char* SAVE_FILENAME = "save.json";

void SaveGame() {
    std::ofstream fout(SAVE_FILENAME);
    if (fout.is_open()) {
        fout << g_game.serialize_to_json();
        fout.close();
    }
}

void LoadGame() {
    std::ifstream fin(SAVE_FILENAME);
    if (fin.is_open()) {
        std::string json_str((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
        fin.close();
        if (!json_str.empty()) {
            g_game.deserialize_from_json(json_str);
        }
    }
}

void StartNewGameThread(HWND hwnd, Difficulty diff) {
    if (g_generating) return;
    g_generating = true;
    g_ui.is_loading = true;
    g_ui.show_esc_menu = false;
    InvalidateRect(hwnd, NULL, FALSE);

    std::thread([hwnd, diff]() {
        SamuraiSudoku new_game;
        new_game.generate_puzzle(diff);

        g_game = new_game;
        g_generating = false;
        g_ui.is_loading = false;
        g_ui.selected_r = -1;
        g_ui.selected_c = -1;
        g_ui.show_number_popup = false;

        SaveGame();
        PostMessage(hwnd, WM_USER + 1, 0, 0); // Notify UI thread done
    }).detach();
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CREATE:
        SetTimer(hwnd, TIMER_ID, 1000, NULL);
        LoadGame();
        if (g_game.elapsed_seconds == 0) {
            // First run, find initial valid selection
            for (int r = 0; r < 21; ++r) {
                for (int c = 0; c < 21; ++c) {
                    if (SamuraiSudoku::is_valid_cell(r, c)) {
                        g_ui.selected_r = r;
                        g_ui.selected_c = c;
                        break;
                    }
                }
                if (g_ui.selected_r != -1) break;
            }
        }
        break;

    case WM_TIMER:
        if (wParam == TIMER_ID) {
            if (!g_ui.is_loading && !g_ui.show_esc_menu && !g_game.is_completed()) {
                g_game.elapsed_seconds++;
                InvalidateRect(hwnd, NULL, FALSE);
            }
        }
        break;

    case WM_USER + 1:
        InvalidateRect(hwnd, NULL, FALSE);
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        RECT clientRect;
        GetClientRect(hwnd, &clientRect);
        UIRenderer::Render(hdc, clientRect.right - clientRect.left, clientRect.bottom - clientRect.top, g_game, g_ui);
        EndPaint(hwnd, &ps);
        break;
    }

    case WM_ERASEBKGND:
        return 1; // Handled double buffer in WM_PAINT

    case WM_LBUTTONDOWN: {
        if (g_ui.is_loading) break;

        int x = LOWORD(lParam);
        int y = HIWORD(lParam);

        if (g_ui.show_number_popup) {
            int action = UIRenderer::GetPopupNumberFromPos(x, y, g_ui);
            if (action >= 1 && action <= 9) {
                if (g_ui.note_mode) {
                    g_game.toggle_note(g_ui.popup_r, g_ui.popup_c, action);
                } else {
                    g_game.set_cell_value(g_ui.popup_r, g_ui.popup_c, action);
                }
                SaveGame();
                g_ui.show_number_popup = false;
            } else if (action == 10) { // Clear
                g_game.clear_cell(g_ui.popup_r, g_ui.popup_c);
                SaveGame();
                g_ui.show_number_popup = false;
            } else {
                g_ui.show_number_popup = false;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        int click_r, click_c;
        UIRenderer::GetCellFromPos(x, y, g_ui, click_r, click_c);
        if (click_r != -1 && click_c != -1) {
            if (g_ui.selected_r == click_r && g_ui.selected_c == click_c) {
                // Clicking already selected cell toggles pop-up
                if (!g_game.board[click_r][click_c].is_given) {
                    g_ui.show_number_popup = true;
                    g_ui.popup_r = click_r;
                    g_ui.popup_c = click_c;
                }
            } else {
                g_ui.selected_r = click_r;
                g_ui.selected_c = click_c;
                g_ui.show_number_popup = false;
            }
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }

    case WM_RBUTTONDOWN: {
        if (g_ui.is_loading) break;
        int x = LOWORD(lParam);
        int y = HIWORD(lParam);
        int click_r, click_c;
        UIRenderer::GetCellFromPos(x, y, g_ui, click_r, click_c);
        if (click_r != -1 && click_c != -1 && !g_game.board[click_r][click_c].is_given) {
            g_ui.selected_r = click_r;
            g_ui.selected_c = click_c;
            g_ui.show_number_popup = true;
            g_ui.popup_r = click_r;
            g_ui.popup_c = click_c;
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }

    case WM_KEYDOWN: {
        if (g_ui.is_loading) break;

        // Handle ESC menu shortcuts
        if (g_ui.show_esc_menu) {
            if (wParam == VK_ESCAPE || wParam == '1') {
                g_ui.show_esc_menu = false;
            } else if (wParam == '2') {
                StartNewGameThread(hwnd, Difficulty::Easy);
            } else if (wParam == '3') {
                StartNewGameThread(hwnd, Difficulty::Medium);
            } else if (wParam == '4') {
                StartNewGameThread(hwnd, Difficulty::Hard);
            } else if (wParam == 'Q') {
                SaveGame();
                PostQuitMessage(0);
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        if (wParam == VK_ESCAPE) {
            if (g_ui.show_number_popup) {
                g_ui.show_number_popup = false;
            } else {
                g_ui.show_esc_menu = true;
            }
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // Toggle note mode with 'N'
        if (wParam == 'N') {
            g_ui.note_mode = !g_ui.note_mode;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // Undo Ctrl+Z
        if ((GetKeyState(VK_CONTROL) & 0x8000) && wParam == 'Z') {
            g_game.undo();
            SaveGame();
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        // Arrow Key Navigation
        int dr = 0, dc = 0;
        if (wParam == VK_LEFT) dc = -1;
        else if (wParam == VK_RIGHT) dc = 1;
        else if (wParam == VK_UP) dr = -1;
        else if (wParam == VK_DOWN) dr = 1;

        if (dr != 0 || dc != 0) {
            int curr_r = g_ui.selected_r < 0 ? 0 : g_ui.selected_r;
            int curr_c = g_ui.selected_c < 0 ? 0 : g_ui.selected_c;

            int next_r = curr_r + dr;
            int next_c = curr_c + dc;

            while (next_r >= 0 && next_r < 21 && next_c >= 0 && next_c < 21) {
                if (SamuraiSudoku::is_valid_cell(next_r, next_c)) {
                    g_ui.selected_r = next_r;
                    g_ui.selected_c = next_c;
                    g_ui.show_number_popup = false;
                    InvalidateRect(hwnd, NULL, FALSE);
                    break;
                }
                next_r += dr;
                next_c += dc;
            }
            break;
        }

        // Delete / Backspace
        if (wParam == VK_DELETE || wParam == VK_BACK) {
            if (g_ui.selected_r >= 0 && g_ui.selected_c >= 0) {
                g_game.clear_cell(g_ui.selected_r, g_ui.selected_c);
                SaveGame();
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }

        // Numbers 1-9 (Main Keyboard & Numpad)
        int num = 0;
        if (wParam >= '1' && wParam <= '9') num = wParam - '0';
        else if (wParam >= VK_NUMPAD1 && wParam <= VK_NUMPAD9) num = wParam - VK_NUMPAD1 + 1;

        if (num >= 1 && num <= 9 && g_ui.selected_r >= 0 && g_ui.selected_c >= 0) {
            bool is_shift = (GetKeyState(VK_SHIFT) & 0x8000) != 0;
            if (g_ui.note_mode || is_shift) {
                g_game.toggle_note(g_ui.selected_r, g_ui.selected_c, num);
            } else {
                g_game.set_cell_value(g_ui.selected_r, g_ui.selected_c, num);
            }
            SaveGame();
            InvalidateRect(hwnd, NULL, FALSE);
        }
        break;
    }

    case WM_CLOSE:
        SaveGame();
        DestroyWindow(hwnd);
        break;

    case WM_DESTROY:
        KillTimer(hwnd, TIMER_ID);
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    const char CLASS_NAME[] = "SamuraiSudokuWindowClass";

    WNDCLASSEXA wc = {};
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);

    RegisterClassExA(&wc);

    // Fullscreen borderless window setup
    int screen_w = GetSystemMetrics(SM_CXSCREEN);
    int screen_h = GetSystemMetrics(SM_CYSCREEN);

    HWND hwnd = CreateWindowExA(
        0,
        CLASS_NAME,
        "Samurai Sudoku",
        WS_POPUP | WS_VISIBLE,
        0, 0, screen_w, screen_h,
        NULL, NULL, hInstance, NULL
    );

    if (hwnd == NULL) return 0;

    ShowWindow(hwnd, SW_SHOWMAXIMIZED);
    UpdateWindow(hwnd);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}
