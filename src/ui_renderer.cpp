#include "ui_renderer.h"
#include <string>
#include <cstdio>

// Color constants
const COLORREF COLOR_BG = RGB(245, 247, 250);
const COLORREF COLOR_HEADER_BG = RGB(40, 44, 52);
const COLORREF COLOR_HEADER_TEXT = RGB(240, 240, 240);
const COLORREF COLOR_CELL_BG = RGB(255, 255, 255);
const COLORREF COLOR_GIVEN_BG = RGB(240, 243, 246);
const COLORREF COLOR_OVERLAP_BG = RGB(232, 240, 254);
const COLORREF COLOR_SELECTED_BG = RGB(187, 222, 251);
const COLORREF COLOR_SAME_NUM_BG = RGB(227, 242, 253);
const COLORREF COLOR_ERROR_BG = RGB(255, 205, 210);

const COLORREF COLOR_TEXT_GIVEN = RGB(20, 20, 20);
const COLORREF COLOR_TEXT_USER = RGB(25, 118, 210);
const COLORREF COLOR_TEXT_ERROR = RGB(211, 47, 47);
const COLORREF COLOR_TEXT_NOTE = RGB(100, 116, 139);

const COLORREF COLOR_GRID_LINE = RGB(210, 215, 220);
const COLORREF COLOR_THICK_LINE = RGB(50, 50, 50);

void UIRenderer::GetCellFromPos(int mouse_x, int mouse_y, const UIState& ui, int& out_r, int& out_c) {
    out_r = -1;
    out_c = -1;
    if (ui.cell_size <= 0) return;

    int rel_x = mouse_x - ui.grid_origin_x;
    int rel_y = mouse_y - ui.grid_origin_y;

    if (rel_x < 0 || rel_y < 0) return;

    int c = rel_x / ui.cell_size;
    int r = rel_y / ui.cell_size;

    if (r >= 0 && r < 21 && c >= 0 && c < 21) {
        if (SamuraiSudoku::is_valid_cell(r, c)) {
            out_r = r;
            out_c = c;
        }
    }
}

int UIRenderer::GetPopupNumberFromPos(int mouse_x, int mouse_y, const UIState& ui) {
    if (!ui.show_number_popup) return 0;
    if (mouse_x < ui.popup_rect.left || mouse_x > ui.popup_rect.right ||
        mouse_y < ui.popup_rect.top || mouse_y > ui.popup_rect.bottom) {
        return 0;
    }

    int btn_w = (ui.popup_rect.right - ui.popup_rect.left) / 3;
    int btn_h = (ui.popup_rect.bottom - ui.popup_rect.top - 30) / 3;

    int rel_x = mouse_x - ui.popup_rect.left;
    int rel_y = mouse_y - (ui.popup_rect.top + 30);

    if (rel_y < 0) {
        if (rel_x < (ui.popup_rect.right - ui.popup_rect.left) / 2) return 10; // Clear
        else return 11; // Close
    }

    int col = rel_x / btn_w;
    int row = rel_y / btn_h;

    if (row >= 0 && row < 3 && col >= 0 && col < 3) {
        return row * 3 + col + 1;
    }

    return 0;
}

void UIRenderer::Render(HDC hdc, int width, int height, const SamuraiSudoku& game, const UIState& ui) {
    // Double buffering setup
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    // Fill background
    HBRUSH bgBrush = CreateSolidBrush(COLOR_BG);
    RECT fullScreenRect = {0, 0, width, height};
    FillRect(memDC, &fullScreenRect, bgBrush);
    DeleteObject(bgBrush);

    // 1. Header Bar (Top 60px)
    int header_height = 60;
    RECT headerRect = {0, 0, width, header_height};
    HBRUSH headerBrush = CreateSolidBrush(COLOR_HEADER_BG);
    FillRect(memDC, &headerRect, headerBrush);
    DeleteObject(headerBrush);

    SetBkMode(memDC, TRANSPARENT);
    SetTextColor(memDC, COLOR_HEADER_TEXT);

    HFONT titleFont = CreateFontA(24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, titleFont);

    TextOutA(memDC, 20, 15, "SAMURAI SUDOKU", 14);

    // Header info (Difficulty, Timer, Errors, Note Mode)
    HFONT infoFont = CreateFontA(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    SelectObject(memDC, infoFont);

    char diff_str[32];
    const char* diff_names[] = {"Easy", "Medium", "Hard"};
    snprintf(diff_str, sizeof(diff_str), "Difficulty: %s", diff_names[static_cast<int>(game.difficulty)]);
    TextOutA(memDC, 250, 18, diff_str, (int)strlen(diff_str));

    int mins = game.elapsed_seconds / 60;
    int secs = game.elapsed_seconds % 60;
    char time_str[32];
    snprintf(time_str, sizeof(time_str), "Time: %02d:%02d", mins, secs);
    TextOutA(memDC, 450, 18, time_str, (int)strlen(time_str));

    char err_str[32];
    snprintf(err_str, sizeof(err_str), "Errors: %d", game.error_count);
    TextOutA(memDC, 600, 18, err_str, (int)strlen(err_str));

    const char* mode_str = ui.note_mode ? "[Mode: NOTES (Shift or 'N')]" : "[Mode: NORMAL]";
    TextOutA(memDC, 750, 18, mode_str, (int)strlen(mode_str));

    TextOutA(memDC, width - 220, 18, "[ESC] Menu | [Ctrl+Z] Undo", 26);

    DeleteObject(titleFont);

    // 2. Grid Dimensions
    int available_height = height - header_height - 40;
    int available_width = width - 40;
    int grid_pixel_size = (available_height < available_width ? available_height : available_width);
    int cell_sz = grid_pixel_size / 21;
    if (cell_sz < 15) cell_sz = 15;
    grid_pixel_size = cell_sz * 21;

    int origin_x = (width - grid_pixel_size) / 2;
    int origin_y = header_height + (available_height - grid_pixel_size) / 2 + 10;

    UIState* mutable_ui = const_cast<UIState*>(&ui);
    mutable_ui->cell_size = cell_sz;
    mutable_ui->grid_origin_x = origin_x;
    mutable_ui->grid_origin_y = origin_y;

    int selected_val = 0;
    if (ui.selected_r >= 0 && ui.selected_c >= 0) {
        selected_val = game.board[ui.selected_r][ui.selected_c].value;
    }

    HFONT cellNumFont = CreateFontA(cell_sz * 6 / 10, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT noteFont = CreateFontA(cell_sz * 28 / 100, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    // 3. Draw Grid Cells
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (!SamuraiSudoku::is_valid_cell(r, c)) continue;

            int cell_left = origin_x + c * cell_sz;
            int cell_top = origin_y + r * cell_sz;
            RECT cRect = {cell_left, cell_top, cell_left + cell_sz, cell_top + cell_sz};

            int grids[2];
            int g_count = 0;
            SamuraiSudoku::get_grid_memberships(r, c, grids, g_count);

            COLORREF bg_color = COLOR_CELL_BG;
            if (game.board[r][c].is_error) {
                bg_color = COLOR_ERROR_BG;
            } else if (r == ui.selected_r && c == ui.selected_c) {
                bg_color = COLOR_SELECTED_BG;
            } else if (selected_val > 0 && game.board[r][c].value == selected_val) {
                bg_color = COLOR_SAME_NUM_BG;
            } else if (g_count > 1) {
                bg_color = COLOR_OVERLAP_BG;
            } else if (game.board[r][c].is_given) {
                bg_color = COLOR_GIVEN_BG;
            }

            HBRUSH cBrush = CreateSolidBrush(bg_color);
            FillRect(memDC, &cRect, cBrush);
            DeleteObject(cBrush);

            HPEN thinPen = CreatePen(PS_SOLID, 1, COLOR_GRID_LINE);
            HPEN oldPen = (HPEN)SelectObject(memDC, thinPen);

            MoveToEx(memDC, cell_left, cell_top, NULL);
            LineTo(memDC, cell_left + cell_sz, cell_top);
            LineTo(memDC, cell_left + cell_sz, cell_top + cell_sz);
            LineTo(memDC, cell_left, cell_top + cell_sz);
            LineTo(memDC, cell_left, cell_top);

            SelectObject(memDC, oldPen);
            DeleteObject(thinPen);

            const auto& cell = game.board[r][c];
            if (cell.value != 0) {
                SelectObject(memDC, cellNumFont);
                if (cell.is_error) SetTextColor(memDC, COLOR_TEXT_ERROR);
                else if (cell.is_given) SetTextColor(memDC, COLOR_TEXT_GIVEN);
                else SetTextColor(memDC, COLOR_TEXT_USER);

                char val_str[4];
                snprintf(val_str, sizeof(val_str), "%d", cell.value);
                SIZE text_sz;
                GetTextExtentPoint32A(memDC, val_str, 1, &text_sz);
                TextOutA(memDC, cell_left + (cell_sz - text_sz.cx) / 2, cell_top + (cell_sz - text_sz.cy) / 2, val_str, 1);
            } else if (cell.notes != 0) {
                SelectObject(memDC, noteFont);
                SetTextColor(memDC, COLOR_TEXT_NOTE);

                int n_sz = cell_sz / 3;
                for (int n = 1; n <= 9; ++n) {
                    if (cell.notes & (1 << n)) {
                        int nr = (n - 1) / 3;
                        int nc = (n - 1) % 3;
                        char n_str[2] = { (char)('0' + n), '\0' };
                        TextOutA(memDC, cell_left + nc * n_sz + 2, cell_top + nr * n_sz + 1, n_str, 1);
                    }
                }
            }
        }
    }

    // 4. Draw Thick 3x3 Block Borders for 5 Sub-Grids
    HPEN thickPen = CreatePen(PS_SOLID, 3, COLOR_THICK_LINE);
    HPEN oldPen = (HPEN)SelectObject(memDC, thickPen);

    for (int g = 0; g < 5; ++g) {
        int base_r = GRID_OFFSETS[g].r;
        int base_c = GRID_OFFSETS[g].c;

        for (int br = 0; br <= 9; br += 3) {
            int y = origin_y + (base_r + br) * cell_sz;
            int x1 = origin_x + base_c * cell_sz;
            int x2 = origin_x + (base_c + 9) * cell_sz;
            MoveToEx(memDC, x1, y, NULL);
            LineTo(memDC, x2, y);
        }

        for (int bc = 0; bc <= 9; bc += 3) {
            int x = origin_x + (base_c + bc) * cell_sz;
            int y1 = origin_y + base_r * cell_sz;
            int y2 = origin_y + (base_r + 9) * cell_sz;
            MoveToEx(memDC, x, y1, NULL);
            LineTo(memDC, x, y2);
        }
    }

    SelectObject(memDC, oldPen);
    DeleteObject(thickPen);

    // 5. Draw Pop-up Number Selector if Active
    if (ui.show_number_popup && ui.popup_r >= 0 && ui.popup_c >= 0) {
        int pop_w = 180;
        int pop_h = 210;
        int pop_x = origin_x + ui.popup_c * cell_sz + cell_sz / 2;
        int pop_y = origin_y + ui.popup_r * cell_sz + cell_sz / 2;

        if (pop_x + pop_w > width - 20) pop_x = width - pop_w - 20;
        if (pop_y + pop_h > height - 20) pop_y = height - pop_h - 20;

        RECT pRect = {pop_x, pop_y, pop_x + pop_w, pop_y + pop_h};
        mutable_ui->popup_rect = pRect;

        HBRUSH shadowBrush = CreateSolidBrush(RGB(180, 180, 180));
        RECT shadowRect = {pop_x + 4, pop_y + 4, pop_x + pop_w + 4, pop_y + pop_h + 4};
        FillRect(memDC, &shadowRect, shadowBrush);
        DeleteObject(shadowBrush);

        HBRUSH popBg = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &pRect, popBg);
        DeleteObject(popBg);

        RECT popHeader = {pop_x, pop_y, pop_x + pop_w, pop_y + 30};
        HBRUSH pHeadBg = CreateSolidBrush(RGB(230, 235, 242));
        FillRect(memDC, &popHeader, pHeadBg);
        DeleteObject(pHeadBg);

        SelectObject(memDC, infoFont);
        SetTextColor(memDC, RGB(30, 30, 30));
        TextOutA(memDC, pop_x + 10, pop_y + 5, "Clear", 5);
        TextOutA(memDC, pop_x + pop_w - 45, pop_y + 5, "Close", 5);

        int btn_w = pop_w / 3;
        int btn_h = (pop_h - 30) / 3;

        HPEN popPen = CreatePen(PS_SOLID, 1, RGB(200, 200, 200));
        SelectObject(memDC, popPen);

        for (int r = 0; r < 3; ++r) {
            for (int c = 0; c < 3; ++c) {
                int num = r * 3 + c + 1;
                int bx = pop_x + c * btn_w;
                int by = pop_y + 30 + r * btn_h;
                RECT bRect = {bx, by, bx + btn_w, by + btn_h};

                FrameRect(memDC, &bRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

                SelectObject(memDC, titleFont);
                char num_str[2] = {(char)('0' + num), '\0'};
                SIZE text_sz;
                GetTextExtentPoint32A(memDC, num_str, 1, &text_sz);
                TextOutA(memDC, bx + (btn_w - text_sz.cx) / 2, by + (btn_h - text_sz.cy) / 2, num_str, 1);
            }
        }
        DeleteObject(popPen);
    }

    // 6. Draw ESC Menu Overlay or Loading Overlay
    if (ui.is_loading) {
        FillRect(memDC, &fullScreenRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

        RECT loadRect = {width / 2 - 200, height / 2 - 60, width / 2 + 200, height / 2 + 60};
        HBRUSH boxBg = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &loadRect, boxBg);
        DeleteObject(boxBg);

        SelectObject(memDC, infoFont);
        SetTextColor(memDC, RGB(20, 20, 20));
        const char* load_msg = "Generating New Samurai Puzzle...";
        SIZE sz;
        GetTextExtentPoint32A(memDC, load_msg, (int)strlen(load_msg), &sz);
        TextOutA(memDC, width / 2 - sz.cx / 2, height / 2 - sz.cy / 2, load_msg, (int)strlen(load_msg));
    } else if (ui.show_esc_menu) {
        RECT menuRect = {width / 2 - 180, height / 2 - 160, width / 2 + 180, height / 2 + 160};
        HBRUSH menuBg = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &menuRect, menuBg);
        DeleteObject(menuBg);

        FrameRect(memDC, &menuRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        SelectObject(memDC, infoFont);
        SetTextColor(memDC, RGB(30, 30, 30));
        TextOutA(memDC, width / 2 - 60, height / 2 - 130, "GAME MENU", 9);

        TextOutA(memDC, width / 2 - 120, height / 2 - 70, "[1] Resume Game", 15);
        TextOutA(memDC, width / 2 - 120, height / 2 - 30, "[2] New Game (Easy)", 19);
        TextOutA(memDC, width / 2 - 120, height / 2 + 10, "[3] New Game (Medium)", 21);
        TextOutA(memDC, width / 2 - 120, height / 2 + 50, "[4] New Game (Hard)", 19);
        TextOutA(memDC, width / 2 - 120, height / 2 + 90, "[ESC / Q] Quit Game", 19);
    }

    DeleteObject(infoFont);
    DeleteObject(cellNumFont);
    DeleteObject(noteFont);

    // BitBlt double buffer to screen
    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
