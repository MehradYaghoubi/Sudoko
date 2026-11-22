#include "../src/main.cpp"
#include <cassert>
#include <iostream>

static void click(HWND hwnd, RECT r) {
    WndProc(hwnd, WM_LBUTTONDOWN, 0, MAKELPARAM((r.left + r.right) / 2, (r.top + r.bottom) / 2));
}

int main() {
    // Exercise the real message handler with an isolated save and an invisible test window.
    SAVE_FILENAME = ".qa-preview/input-test-save.json";
    WNDCLASSA wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = "SudokuInputTest";
    assert(RegisterClassA(&wc));
    HWND hwnd = CreateWindowA(wc.lpszClassName, "Test", WS_OVERLAPPEDWINDOW,
                              0, 0, 1000, 760, nullptr, nullptr, wc.hInstance, nullptr);
    assert(hwnd);
    RECT client;
    GetClientRect(hwnd, &client);
    UIRenderer::UpdateLayout(client.right, client.bottom, g_ui);
    g_game = SamuraiSudoku();
    g_game.board[0][0].solution = 1;
    g_ui.screen = AppScreen::InGame;
    g_ui.selected_r = g_ui.selected_c = 0;
    WndProc(hwnd, WM_KEYDOWN, '2', 0);
    assert(g_game.board[0][0].value == 2 && !g_game.board[0][0].is_error);
    click(hwnd, g_ui.btn_check);
    assert(g_game.error_count == 1 && g_game.check_revealed);
    WndProc(hwnd, WM_KEYDOWN, '1', 0);
    assert(!g_game.check_revealed && g_game.error_count == 0);
    WndProc(hwnd, WM_KEYDOWN, 'C', 0);
    assert(g_game.check_revealed && g_game.error_count == 0);
    WndProc(hwnd, WM_KEYDOWN, VK_ESCAPE, 0);
    assert(g_ui.show_esc_menu);
    click(hwnd, g_ui.btn_erase);
    assert(g_game.board[0][0].value == 1 && g_ui.show_esc_menu);
    WndProc(hwnd, WM_RBUTTONDOWN, 0, MAKELPARAM(g_ui.grid_origin_x + 1, g_ui.grid_origin_y + 1));
    assert(!g_ui.show_number_popup);
    click(hwnd, g_ui.pause_buttons[0]);
    assert(!g_ui.show_esc_menu);
    click(hwnd, g_ui.btn_check_mode);
    assert(!g_game.manual_check);
    WndProc(hwnd, WM_KEYDOWN, '2', 0);
    assert(g_game.error_count == 1);
    click(hwnd, g_ui.btn_check_mode);
    assert(g_game.manual_check && g_game.error_count == 0);
    g_ui.show_number_popup = true;
    g_ui.popup_r = g_ui.popup_c = 0;
    HDC dc = GetDC(hwnd);
    UIRenderer::Render(dc, 984, 721, g_game, g_ui);
    ReleaseDC(hwnd, dc);
    RECT popup = g_ui.popup_rect;
    int digit = UIRenderer::GetPopupNumberFromPos(popup.left + 1, popup.top + 33, g_ui);
    assert(digit == 1);
    WndProc(hwnd, WM_LBUTTONDOWN, 0, MAKELPARAM(popup.left + 1, popup.top + 33));
    assert(g_game.board[0][0].value == 1 && !g_game.check_revealed && !g_ui.show_number_popup);
    DestroyWindow(hwnd);
    std::cout << "Real input handler: manual/automatic checking, keyboard, popup and modal blocking passed.\n";
}
