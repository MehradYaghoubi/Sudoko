#ifndef UI_RENDERER_H
#define UI_RENDERER_H

#include <windows.h>
#include "sudoku_engine.h"

struct UIState {
    int selected_r = -1;
    int selected_c = -1;
    bool note_mode = false;
    bool show_esc_menu = false;
    bool show_new_game_modal = false;
    bool is_loading = false;
    bool show_number_popup = false;
    int popup_r = -1;
    int popup_c = -1;
    RECT popup_rect = {0, 0, 0, 0};
    RECT grid_rect = {0, 0, 0, 0};
    int cell_size = 30;
    int grid_origin_x = 0;
    int grid_origin_y = 0;
};

class UIRenderer {
public:
    static void Render(HDC hdc, int width, int height, const SamuraiSudoku& game, const UIState& ui);
    static void GetCellFromPos(int mouse_x, int mouse_y, const UIState& ui, int& out_r, int& out_c);
    static int GetPopupNumberFromPos(int mouse_x, int mouse_y, const UIState& ui);
};

#endif // UI_RENDERER_H
