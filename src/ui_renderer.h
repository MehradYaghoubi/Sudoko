#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include <windows.h>
#include "sudoku_engine.h"

enum class AppScreen {
    MainMenu,
    DifficultySelect,
    InGame,
    ControlsGuide
};

struct UIState {
    AppScreen screen = AppScreen::MainMenu;
    int selected_r = -1;
    int selected_c = -1;
    bool note_mode = false;
    bool senior_mode = true; // High-contrast & enlarged fonts by default
    bool show_esc_menu = false;
    bool is_loading = false;
    bool show_number_popup = false;
    int popup_r = -1;
    int popup_c = -1;
    RECT popup_rect = {0, 0, 0, 0};
    RECT grid_rect = {0, 0, 0, 0};
    int cell_size = 30;
    int grid_origin_x = 0;
    int grid_origin_y = 0;

    // Action button rectangles in game HUD
    RECT btn_undo = {0, 0, 0, 0};
    RECT btn_erase = {0, 0, 0, 0};
    RECT btn_notes = {0, 0, 0, 0};
    RECT btn_reset = {0, 0, 0, 0};
    RECT btn_theme = {0, 0, 0, 0};
    RECT btn_lang = {0, 0, 0, 0};
    RECT menu_buttons[6] = {};
    RECT pause_buttons[5] = {};
    RECT btn_check = {};
    RECT btn_check_mode = {};
    RECT btn_menu = {0, 0, 0, 0};
};

class UIRenderer {
public:
    static void UpdateLayout(int width, int height, UIState& ui);
    static void Render(HDC hdc, int width, int height, const SamuraiSudoku& game, UIState& ui);
    static void GetCellFromPos(int mouse_x, int mouse_y, const UIState& ui, int& out_r, int& out_c);
    static int GetPopupNumberFromPos(int mouse_x, int mouse_y, const UIState& ui);
    static std::wstring Utf8ToWide(const std::string& utf8_str);
};

#endif // UI_RENDERER_H
