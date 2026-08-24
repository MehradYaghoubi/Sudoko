#include "ui_renderer.h"
#include <string>
#include <cstdio>

// Theme Color Definitions
// 1. Modern Theme
const COLORREF MODERN_BG = RGB(245, 247, 250);
const COLORREF MODERN_HEADER_BG = RGB(40, 44, 52);
const COLORREF MODERN_HEADER_TEXT = RGB(240, 240, 240);
const COLORREF MODERN_CELL_BG = RGB(255, 255, 255);
const COLORREF MODERN_GIVEN_BG = RGB(240, 243, 246);
const COLORREF MODERN_OVERLAP_BG = RGB(232, 240, 254);
const COLORREF MODERN_SELECTED_BG = RGB(187, 222, 251);
const COLORREF MODERN_SAME_NUM_BG = RGB(227, 242, 253);
const COLORREF MODERN_ERROR_BG = RGB(255, 205, 210);

const COLORREF MODERN_TEXT_GIVEN = RGB(20, 20, 20);
const COLORREF MODERN_TEXT_USER = RGB(25, 118, 210);
const COLORREF MODERN_TEXT_ERROR = RGB(211, 47, 47);
const COLORREF MODERN_TEXT_NOTE = RGB(100, 116, 139);

// 2. Senior / High-Contrast Theme (High readability, deep blues, sharp contrast)
const COLORREF SENIOR_BG = RGB(255, 255, 255);
const COLORREF SENIOR_HEADER_BG = RGB(15, 23, 42); // Deep navy
const COLORREF SENIOR_HEADER_TEXT = RGB(255, 255, 255);
const COLORREF SENIOR_CELL_BG = RGB(255, 255, 255);
const COLORREF SENIOR_GIVEN_BG = RGB(226, 232, 240);
const COLORREF SENIOR_OVERLAP_BG = RGB(254, 240, 138); // Clear high-contrast yellow tint
const COLORREF SENIOR_SELECTED_BG = RGB(147, 197, 253); // Bright sky blue
const COLORREF SENIOR_SAME_NUM_BG = RGB(191, 219, 254);
const COLORREF SENIOR_ERROR_BG = RGB(254, 202, 202);

const COLORREF SENIOR_TEXT_GIVEN = RGB(0, 0, 0);       // Pure black bold
const COLORREF SENIOR_TEXT_USER = RGB(2, 132, 199);    // Crisp deep blue
const COLORREF SENIOR_TEXT_ERROR = RGB(185, 28, 28);   // Sharp red
const COLORREF SENIOR_TEXT_NOTE = RGB(51, 65, 85);

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
    int btn_h = (ui.popup_rect.bottom - ui.popup_rect.top - 40) / 3;

    int rel_x = mouse_x - ui.popup_rect.left;
    int rel_y = mouse_y - (ui.popup_rect.top + 40);

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

static void DrawButton(HDC hdc, RECT rect, const char* text, COLORREF bg, COLORREF textColor, HFONT font) {
    HBRUSH brush = CreateSolidBrush(bg);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    SelectObject(hdc, font);
    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);

    SIZE sz;
    GetTextExtentPoint32A(hdc, text, (int)strlen(text), &sz);
    int x = rect.left + (rect.right - rect.left - sz.cx) / 2;
    int y = rect.top + (rect.bottom - rect.top - sz.cy) / 2;
    TextOutA(hdc, x, y, text, (int)strlen(text));
}

void UIRenderer::Render(HDC hdc, int width, int height, const SamuraiSudoku& game, const UIState& ui) {
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    bool senior = ui.senior_mode;
    COLORREF bg_clr = senior ? SENIOR_BG : MODERN_BG;
    COLORREF head_bg = senior ? SENIOR_HEADER_BG : MODERN_HEADER_BG;
    COLORREF head_txt = senior ? SENIOR_HEADER_TEXT : MODERN_HEADER_TEXT;

    HBRUSH bgBrush = CreateSolidBrush(bg_clr);
    RECT fullScreenRect = {0, 0, width, height};
    FillRect(memDC, &fullScreenRect, bgBrush);
    DeleteObject(bgBrush);

    HFONT titleFont = CreateFontA(senior ? 32 : 26, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT buttonFont = CreateFontA(senior ? 22 : 18, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
    HFONT infoFont = CreateFontA(senior ? 22 : 18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    // -------------------------------------------------------------
    // SCREEN 1: MAIN MENU
    // -------------------------------------------------------------
    if (ui.screen == AppScreen::MainMenu) {
        SelectObject(memDC, titleFont);
        SetTextColor(memDC, senior ? RGB(15, 23, 42) : RGB(30, 40, 60));
        SetBkMode(memDC, TRANSPARENT);

        const char* main_title = "SAMURAI SUDOKU";
        SIZE sz;
        GetTextExtentPoint32A(memDC, main_title, (int)strlen(main_title), &sz);
        TextOutA(memDC, width / 2 - sz.cx / 2, height / 6, main_title, (int)strlen(main_title));

        int btn_w = senior ? 360 : 300;
        int btn_h = senior ? 60 : 50;
        int start_y = height / 3;
        int gap = senior ? 25 : 20;

        RECT r1 = {width / 2 - btn_w / 2, start_y, width / 2 + btn_w / 2, start_y + btn_h};
        RECT r2 = {width / 2 - btn_w / 2, start_y + (btn_h + gap), width / 2 + btn_w / 2, start_y + (btn_h + gap) + btn_h};
        RECT r3 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 2, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 2 + btn_h};
        RECT r4 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 3, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 3 + btn_h};
        RECT r5 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 4, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 4 + btn_h};

        COLORREF btn_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF btn_txt = RGB(15, 23, 42);

        DrawButton(memDC, r1, "1. Continue Game", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r2, "2. New Game", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r3, senior ? "3. Theme Mode: SENIOR (High-Contrast)" : "3. Theme Mode: MODERN", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r4, "4. Controls Guide", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r5, "5. Quit Game", btn_bg, btn_txt, buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 2: DIFFICULTY SELECTION
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::DifficultySelect) {
        SelectObject(memDC, titleFont);
        SetTextColor(memDC, senior ? RGB(15, 23, 42) : RGB(30, 40, 60));
        SetBkMode(memDC, TRANSPARENT);

        const char* diff_title = "SELECT DIFFICULTY LEVEL";
        SIZE sz;
        GetTextExtentPoint32A(memDC, diff_title, (int)strlen(diff_title), &sz);
        TextOutA(memDC, width / 2 - sz.cx / 2, height / 6, diff_title, (int)strlen(diff_title));

        int btn_w = senior ? 360 : 300;
        int btn_h = senior ? 55 : 48;
        int start_y = height / 3;
        int gap = senior ? 20 : 16;

        RECT r1 = {width / 2 - btn_w / 2, start_y, width / 2 + btn_w / 2, start_y + btn_h};
        RECT r2 = {width / 2 - btn_w / 2, start_y + (btn_h + gap), width / 2 + btn_w / 2, start_y + (btn_h + gap) + btn_h};
        RECT r3 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 2, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 2 + btn_h};
        RECT r4 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 3, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 3 + btn_h};
        RECT r5 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 4, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 4 + btn_h};
        RECT r6 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 5, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 5 + btn_h};

        COLORREF btn_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF btn_txt = RGB(15, 23, 42);

        DrawButton(memDC, r1, "1. Easy", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r2, "2. Medium", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r3, "3. Hard", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r4, "4. Expert", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r5, "5. Master (Monster)", btn_bg, btn_txt, buttonFont);
        DrawButton(memDC, r6, "< Back to Main Menu", RGB(254, 226, 226), RGB(153, 27, 27), buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 3: CONTROLS GUIDE
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::ControlsGuide) {
        SelectObject(memDC, titleFont);
        SetTextColor(memDC, senior ? RGB(15, 23, 42) : RGB(30, 40, 60));
        SetBkMode(memDC, TRANSPARENT);

        const char* guide_title = "CONTROLS & GUIDE";
        SIZE sz;
        GetTextExtentPoint32A(memDC, guide_title, (int)strlen(guide_title), &sz);
        TextOutA(memDC, width / 2 - sz.cx / 2, height / 8, guide_title, (int)strlen(guide_title));

        SelectObject(memDC, infoFont);
        int y = height / 4;
        int step = senior ? 40 : 32;

        const char* lines[] = {
            "Mouse: Click cell to select. Click again or Right-click to open Pop-up keypad.",
            "Arrow Keys: Navigate selected cell across the 5 overlapping grids.",
            "Numbers 1-9: Enter digit into selected cell.",
            "Delete / Backspace: Clear selected cell.",
            "Shift + 1-9 or 'N' Key: Toggle Pencil Notes mode.",
            "Ctrl + Z: Undo previous move.",
            "ESC: Open Game Menu.",
            "High-Contrast Mode: Designed for optimal visibility and readability."
        };

        for (int i = 0; i < 8; ++i) {
            GetTextExtentPoint32A(memDC, lines[i], (int)strlen(lines[i]), &sz);
            TextOutA(memDC, width / 2 - sz.cx / 2, y + i * step, lines[i], (int)strlen(lines[i]));
        }

        RECT r_back = {width / 2 - 150, y + 9 * step, width / 2 + 150, y + 9 * step + 50};
        DrawButton(memDC, r_back, "< Back to Main Menu", RGB(224, 231, 255), RGB(15, 23, 42), buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 4: IN-GAME HUD & BOARD
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::InGame) {
        // Top Header Bar (Height 70px)
        int header_height = senior ? 70 : 60;
        RECT headerRect = {0, 0, width, header_height};
        HBRUSH headerBrush = CreateSolidBrush(head_bg);
        FillRect(memDC, &headerRect, headerBrush);
        DeleteObject(headerBrush);

        SetBkMode(memDC, TRANSPARENT);
        SetTextColor(memDC, head_txt);

        SelectObject(memDC, titleFont);
        TextOutA(memDC, 20, senior ? 15 : 12, "SAMURAI SUDOKU", 14);

        SelectObject(memDC, infoFont);
        const char* diff_names[] = {"Easy", "Medium", "Hard", "Expert", "Master"};
        char diff_str[32];
        snprintf(diff_str, sizeof(diff_str), "Diff: %s", diff_names[static_cast<int>(game.difficulty)]);
        TextOutA(memDC, 300, senior ? 20 : 18, diff_str, (int)strlen(diff_str));

        int mins = game.elapsed_seconds / 60;
        int secs = game.elapsed_seconds % 60;
        char time_str[32];
        snprintf(time_str, sizeof(time_str), "Time: %02d:%02d", mins, secs);
        TextOutA(memDC, 480, senior ? 20 : 18, time_str, (int)strlen(time_str));

        char err_str[32];
        snprintf(err_str, sizeof(err_str), "Errors: %d", game.error_count);
        TextOutA(memDC, 650, senior ? 20 : 18, err_str, (int)strlen(err_str));

        // Bottom/Side HUD Large Touch Action Buttons (Ideal for Senior/Ease of access)
        int btn_w = senior ? 130 : 110;
        int btn_h = senior ? 50 : 42;
        int hud_y = height - btn_h - 15;
        int total_hud_w = 6 * btn_w + 5 * 15;
        int start_x = (width - total_hud_w) / 2;

        UIState* mutable_ui = const_cast<UIState*>(&ui);
        mutable_ui->btn_undo  = {start_x, hud_y, start_x + btn_w, hud_y + btn_h};
        mutable_ui->btn_erase = {start_x + (btn_w + 15), hud_y, start_x + (btn_w + 15) + btn_w, hud_y + btn_h};
        mutable_ui->btn_notes = {start_x + (btn_w + 15) * 2, hud_y, start_x + (btn_w + 15) * 2 + btn_w, hud_y + btn_h};
        mutable_ui->btn_reset = {start_x + (btn_w + 15) * 3, hud_y, start_x + (btn_w + 15) * 3 + btn_w, hud_y + btn_h};
        mutable_ui->btn_theme = {start_x + (btn_w + 15) * 4, hud_y, start_x + (btn_w + 15) * 4 + btn_w, hud_y + btn_h};
        mutable_ui->btn_menu  = {start_x + (btn_w + 15) * 5, hud_y, start_x + (btn_w + 15) * 5 + btn_w, hud_y + btn_h};

        COLORREF action_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF active_notes_bg = RGB(254, 240, 138);

        DrawButton(memDC, ui.btn_undo, "Undo", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButton(memDC, ui.btn_erase, "Erase", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButton(memDC, ui.btn_notes, ui.note_mode ? "Notes [ON]" : "Notes [OFF]", ui.note_mode ? active_notes_bg : action_bg, RGB(15, 23, 42), buttonFont);
        DrawButton(memDC, ui.btn_reset, "Reset", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButton(memDC, ui.btn_theme, senior ? "Senior UI" : "Modern UI", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButton(memDC, ui.btn_menu, "Menu (ESC)", action_bg, RGB(15, 23, 42), buttonFont);

        // Grid Dimensions calculation
        int available_height = height - header_height - btn_h - 40;
        int available_width = width - 40;
        int grid_pixel_size = (available_height < available_width ? available_height : available_width);
        int cell_sz = grid_pixel_size / 21;
        if (cell_sz < 15) cell_sz = 15;
        grid_pixel_size = cell_sz * 21;

        int origin_x = (width - grid_pixel_size) / 2;
        int origin_y = header_height + (available_height - grid_pixel_size) / 2;

        mutable_ui->cell_size = cell_sz;
        mutable_ui->grid_origin_x = origin_x;
        mutable_ui->grid_origin_y = origin_y;

        int selected_val = 0;
        if (ui.selected_r >= 0 && ui.selected_c >= 0) {
            selected_val = game.board[ui.selected_r][ui.selected_c].value;
        }

        HFONT cellNumFont = CreateFontA(cell_sz * (senior ? 75 : 60) / 100, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        HFONT noteFont = CreateFontA(cell_sz * 28 / 100, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        // Cell background & text colors depending on theme
        COLORREF c_cell_bg = senior ? SENIOR_CELL_BG : MODERN_CELL_BG;
        COLORREF c_given_bg = senior ? SENIOR_GIVEN_BG : MODERN_GIVEN_BG;
        COLORREF c_overlap_bg = senior ? SENIOR_OVERLAP_BG : MODERN_OVERLAP_BG;
        COLORREF c_selected_bg = senior ? SENIOR_SELECTED_BG : MODERN_SELECTED_BG;
        COLORREF c_same_num_bg = senior ? SENIOR_SAME_NUM_BG : MODERN_SAME_NUM_BG;
        COLORREF c_error_bg = senior ? SENIOR_ERROR_BG : MODERN_ERROR_BG;

        COLORREF c_txt_given = senior ? SENIOR_TEXT_GIVEN : MODERN_TEXT_GIVEN;
        COLORREF c_txt_user = senior ? SENIOR_TEXT_USER : MODERN_TEXT_USER;
        COLORREF c_txt_error = senior ? SENIOR_TEXT_ERROR : MODERN_TEXT_ERROR;

        // Render Cells
        for (int r = 0; r < 21; ++r) {
            for (int c = 0; c < 21; ++c) {
                if (!SamuraiSudoku::is_valid_cell(r, c)) continue;

                int cell_left = origin_x + c * cell_sz;
                int cell_top = origin_y + r * cell_sz;
                RECT cRect = {cell_left, cell_top, cell_left + cell_sz, cell_top + cell_sz};

                int grids[2];
                int g_count = 0;
                SamuraiSudoku::get_grid_memberships(r, c, grids, g_count);

                COLORREF bg = c_cell_bg;
                if (game.board[r][c].is_error) {
                    bg = c_error_bg;
                } else if (r == ui.selected_r && c == ui.selected_c) {
                    bg = c_selected_bg;
                } else if (selected_val > 0 && game.board[r][c].value == selected_val) {
                    bg = c_same_num_bg;
                } else if (g_count > 1) {
                    bg = c_overlap_bg;
                } else if (game.board[r][c].is_given) {
                    bg = c_given_bg;
                }

                HBRUSH cBrush = CreateSolidBrush(bg);
                FillRect(memDC, &cRect, cBrush);
                DeleteObject(cBrush);

                HPEN thinPen = CreatePen(PS_SOLID, 1, RGB(180, 190, 200));
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
                    if (cell.is_error) SetTextColor(memDC, c_txt_error);
                    else if (cell.is_given) SetTextColor(memDC, c_txt_given);
                    else SetTextColor(memDC, c_txt_user);

                    char val_str[4];
                    snprintf(val_str, sizeof(val_str), "%d", cell.value);
                    SIZE text_sz;
                    GetTextExtentPoint32A(memDC, val_str, 1, &text_sz);
                    TextOutA(memDC, cell_left + (cell_sz - text_sz.cx) / 2, cell_top + (cell_sz - text_sz.cy) / 2, val_str, 1);
                } else if (cell.notes != 0) {
                    SelectObject(memDC, noteFont);
                    SetTextColor(memDC, senior ? SENIOR_TEXT_NOTE : MODERN_TEXT_NOTE);

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

        // Draw Thick Sub-Grid Borders
        HPEN thickPen = CreatePen(PS_SOLID, senior ? 4 : 3, RGB(15, 23, 42));
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

        // Draw Pop-up Keypad if active
        if (ui.show_number_popup && ui.popup_r >= 0 && ui.popup_c >= 0) {
            int pop_w = senior ? 220 : 190;
            int pop_h = senior ? 250 : 220;
            int pop_x = origin_x + ui.popup_c * cell_sz + cell_sz / 2;
            int pop_y = origin_y + ui.popup_r * cell_sz + cell_sz / 2;

            if (pop_x + pop_w > width - 20) pop_x = width - pop_w - 20;
            if (pop_y + pop_h > height - 20) pop_y = height - pop_h - 20;

            RECT pRect = {pop_x, pop_y, pop_x + pop_w, pop_y + pop_h};
            mutable_ui->popup_rect = pRect;

            HBRUSH popBg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &pRect, popBg);
            DeleteObject(popBg);
            FrameRect(memDC, &pRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

            RECT popHeader = {pop_x, pop_y, pop_x + pop_w, pop_y + 40};
            HBRUSH pHeadBg = CreateSolidBrush(RGB(226, 232, 240));
            FillRect(memDC, &popHeader, pHeadBg);
            DeleteObject(pHeadBg);

            SelectObject(memDC, buttonFont);
            SetTextColor(memDC, RGB(15, 23, 42));
            TextOutA(memDC, pop_x + 15, pop_y + 8, "Erase", 5);
            TextOutA(memDC, pop_x + pop_w - 60, pop_y + 8, "Close", 5);

            int btn_w_p = pop_w / 3;
            int btn_h_p = (pop_h - 40) / 3;

            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    int num = r * 3 + c + 1;
                    int bx = pop_x + c * btn_w_p;
                    int by = pop_y + 40 + r * btn_h_p;
                    RECT bRect = {bx, by, bx + btn_w_p, by + btn_h_p};

                    FrameRect(memDC, &bRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

                    SelectObject(memDC, titleFont);
                    char num_str[2] = {(char)('0' + num), '\0'};
                    SIZE text_sz;
                    GetTextExtentPoint32A(memDC, num_str, 1, &text_sz);
                    TextOutA(memDC, bx + (btn_w_p - text_sz.cx) / 2, by + (btn_h_p - text_sz.cy) / 2, num_str, 1);
                }
            }
        }

        // Draw In-game ESC Menu Overlay
        if (ui.show_esc_menu) {
            RECT menuRect = {width / 2 - 200, height / 2 - 180, width / 2 + 200, height / 2 + 180};
            HBRUSH menuBg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &menuRect, menuBg);
            DeleteObject(menuBg);

            FrameRect(memDC, &menuRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

            SelectObject(memDC, titleFont);
            SetTextColor(memDC, RGB(15, 23, 42));
            TextOutA(memDC, width / 2 - 70, height / 2 - 150, "GAME MENU", 9);

            SelectObject(memDC, buttonFont);
            TextOutA(memDC, width / 2 - 140, height / 2 - 90, "[1] Resume Game", 15);
            TextOutA(memDC, width / 2 - 140, height / 2 - 45, "[2] Restart Current Game", 24);
            TextOutA(memDC, width / 2 - 140, height / 2 + 0, "[3] New Game", 12);
            TextOutA(memDC, width / 2 - 140, height / 2 + 45, "[4] Toggle Senior/Modern UI", 27);
            TextOutA(memDC, width / 2 - 140, height / 2 + 90, "[5 / ESC] Main Menu", 19);
        }

        DeleteObject(cellNumFont);
        DeleteObject(noteFont);
    }

    // Draw Loading Overlay on top if puzzle is generating
    if (ui.is_loading) {
        FillRect(memDC, &fullScreenRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

        RECT loadRect = {width / 2 - 220, height / 2 - 70, width / 2 + 220, height / 2 + 70};
        HBRUSH boxBg = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &loadRect, boxBg);
        DeleteObject(boxBg);
        FrameRect(memDC, &loadRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        SelectObject(memDC, buttonFont);
        SetTextColor(memDC, RGB(15, 23, 42));
        const char* load_msg = "Generating Samurai Sudoku Puzzle...";
        SIZE sz;
        GetTextExtentPoint32A(memDC, load_msg, (int)strlen(load_msg), &sz);
        TextOutA(memDC, width / 2 - sz.cx / 2, height / 2 - sz.cy / 2, load_msg, (int)strlen(load_msg));
    }

    DeleteObject(titleFont);
    DeleteObject(buttonFont);
    DeleteObject(infoFont);

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
