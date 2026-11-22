#include "ui_renderer.h"
#include <string>
#include <cstdio>
#include <vector>
#include <algorithm>

// Theme Color Definitions
const COLORREF MODERN_BG = RGB(246, 248, 252);
const COLORREF MODERN_HEADER_BG = RGB(20, 31, 51);
const COLORREF MODERN_HEADER_TEXT = RGB(248, 250, 252);
const COLORREF MODERN_CELL_BG = RGB(255, 255, 255);
const COLORREF MODERN_GIVEN_BG = RGB(236, 241, 247);
const COLORREF MODERN_OVERLAP_BG = RGB(226, 239, 250);
const COLORREF MODERN_SELECTED_BG = RGB(153, 218, 203);
const COLORREF MODERN_SAME_NUM_BG = RGB(224, 242, 241);
const COLORREF MODERN_ERROR_BG = RGB(255, 205, 210);

const COLORREF MODERN_TEXT_GIVEN = RGB(20, 20, 20);
const COLORREF MODERN_TEXT_USER = RGB(12, 87, 81);
const COLORREF MODERN_TEXT_ERROR = RGB(175, 25, 25);
const COLORREF MODERN_TEXT_NOTE = RGB(71, 85, 105);

// Senior / High-Contrast Theme
const COLORREF SENIOR_BG = RGB(255, 255, 255);
const COLORREF SENIOR_HEADER_BG = RGB(15, 23, 42);
const COLORREF SENIOR_HEADER_TEXT = RGB(255, 255, 255);
const COLORREF SENIOR_CELL_BG = RGB(255, 255, 255);
const COLORREF SENIOR_GIVEN_BG = RGB(226, 232, 240);
const COLORREF SENIOR_OVERLAP_BG = RGB(235, 242, 252);
const COLORREF SENIOR_SELECTED_BG = RGB(147, 197, 253);
const COLORREF SENIOR_SAME_NUM_BG = RGB(191, 219, 254);
const COLORREF SENIOR_ERROR_BG = RGB(254, 202, 202);

const COLORREF SENIOR_TEXT_GIVEN = RGB(0, 0, 0);
const COLORREF SENIOR_TEXT_USER = RGB(3, 75, 125);
const COLORREF SENIOR_TEXT_ERROR = RGB(175, 25, 25);
const COLORREF SENIOR_TEXT_NOTE = RGB(51, 65, 85);

std::wstring UIRenderer::Utf8ToWide(const std::string& utf8_str) {
    if (utf8_str.empty()) return L"";
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &utf8_str[0], (int)utf8_str.size(), NULL, 0);
    std::wstring wstrTo(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, &utf8_str[0], (int)utf8_str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

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
    if (mouse_x < ui.popup_rect.left || mouse_x >= ui.popup_rect.right ||
        mouse_y < ui.popup_rect.top || mouse_y >= ui.popup_rect.bottom) {
        return 0;
    }

    int header_h = 32;
    int btn_w = (ui.popup_rect.right - ui.popup_rect.left) / 3;
    int btn_h = (ui.popup_rect.bottom - ui.popup_rect.top - header_h) / 3;

    int rel_x = mouse_x - ui.popup_rect.left;
    int rel_y = mouse_y - (ui.popup_rect.top + header_h);

    if (rel_y < 0) {
        if (rel_x < (ui.popup_rect.right - ui.popup_rect.left) / 2) return 10; // Clear / Erase
        else return 11; // Close
    }

    int col = rel_x / btn_w;
    int row = rel_y / btn_h;

    if (row >= 0 && row < 3 && col >= 0 && col < 3) {
        return row * 3 + col + 1;
    }

    return 0;
}

static void DrawTextW_Centered(HDC hdc, RECT rect, const std::wstring& wtext, COLORREF textColor, HFONT font) {
    HGDIOBJ oldFont = SelectObject(hdc, font);
    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, wtext.c_str(), (int)wtext.size(), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE | DT_NOPREFIX | DT_END_ELLIPSIS);
    SelectObject(hdc, oldFont);
}

static void DrawButtonW(HDC hdc, RECT rect, const std::wstring& text, COLORREF bg, COLORREF textColor, HFONT font) {
    HBRUSH brush = CreateSolidBrush(bg);
    HPEN pen = CreatePen(PS_SOLID, 1, RGB(203, 213, 225));
    HGDIOBJ oldBrush = SelectObject(hdc, brush);
    HGDIOBJ oldPen = SelectObject(hdc, pen);
    RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 12, 12);
    SelectObject(hdc, oldBrush);
    SelectObject(hdc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);

    RECT textRect = {rect.left + 8, rect.top + 2, rect.right - 8, rect.bottom - 2};
    DrawTextW_Centered(hdc, textRect, text, textColor, font);
}

static void DrawBrandMark(HDC hdc, int center_x, int top) {
    const int board = 18;
    const int x[] = {center_x - 20, center_x + 2, center_x - 9, center_x - 20, center_x + 2};
    const int y[] = {top, top, top + 11, top + 22, top + 22};
    const COLORREF colors[] = {RGB(37, 99, 235), RGB(13, 148, 136), RGB(30, 41, 59), RGB(99, 102, 241), RGB(14, 116, 144)};
    HPEN gridPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
    HGDIOBJ oldPen = SelectObject(hdc, gridPen);
    for (int i = 0; i < 5; ++i) {
        HBRUSH brush = CreateSolidBrush(colors[i]);
        HGDIOBJ oldBrush = SelectObject(hdc, brush);
        RoundRect(hdc, x[i], y[i], x[i] + board, y[i] + board, 3, 3);
        SelectObject(hdc, oldBrush);
        DeleteObject(brush);
        for (int line = 1; line < 3; ++line) {
            MoveToEx(hdc, x[i] + line * board / 3, y[i] + 2, NULL);
            LineTo(hdc, x[i] + line * board / 3, y[i] + board - 2);
            MoveToEx(hdc, x[i] + 2, y[i] + line * board / 3, NULL);
            LineTo(hdc, x[i] + board - 2, y[i] + line * board / 3);
        }
    }
    SelectObject(hdc, oldPen);
    DeleteObject(gridPen);
}

void UIRenderer::UpdateLayout(int width, int height, UIState& ui) {
    const int menu_w = (std::min)(width - 48, 400);
    const int menu_h = ui.senior_mode ? 50 : 44;
    const int start = (std::max)(150, (height - (menu_h + 10) * 6) / 2 + 40);
    for (int i = 0; i < 6; ++i)
        ui.menu_buttons[i] = {width / 2 - menu_w / 2, start + i * (menu_h + 10),
                             width / 2 + menu_w / 2, start + i * (menu_h + 10) + menu_h};
    if (ui.screen == AppScreen::ControlsGuide)
        ui.menu_buttons[0] = {width / 2 - menu_w / 2, height - 70, width / 2 + menu_w / 2, height - 24};
    const bool wide = width >= 960;
    const int board_w = width - 48 - (wide ? 244 : 0);
    const int board_h = height - (wide ? 148 : 260);
    ui.cell_size = (std::max)(1, (std::min)(board_w, board_h) / 21);
    const int grid = ui.cell_size * 21;
    ui.grid_origin_x = (width - grid - (wide ? 244 : 0)) / 2;
    ui.grid_origin_y = (wide ? 100 : 92) + (board_h - grid) / 2;
    ui.grid_rect = {ui.grid_origin_x, ui.grid_origin_y, ui.grid_origin_x + grid, ui.grid_origin_y + grid};
    RECT* buttons[] = {&ui.btn_check, &ui.btn_check_mode, &ui.btn_notes, &ui.btn_undo,
                      &ui.btn_erase, &ui.btn_reset, &ui.btn_theme, &ui.btn_lang, &ui.btn_menu};
    const int bh = wide ? (std::min)(46, (height - 140 - 64) / 9) : 40;
    const int bw = wide ? 216 : (width - 64) / 3;
    const int x = wide ? ui.grid_rect.right + 28 : (width - (3 * bw + 16)) / 2;
    const int y = wide ? 76 + (height - 76 - (9 * bh + 64)) / 2 : ui.grid_rect.bottom + 16;
    for (int i = 0; i < 9; ++i) {
        const int bx = x + (wide ? 0 : i % 3 * (bw + 8));
        const int by = y + (wide ? i * (bh + 8) : i / 3 * (bh + 8));
        *buttons[i] = {bx, by, bx + bw, by + bh};
    }
    const int pause_w = (std::min)(width - 64, 440);
    for (int i = 0; i < 5; ++i)
        ui.pause_buttons[i] = {width / 2 - pause_w / 2, height / 2 - 100 + i * 52,
                              width / 2 + pause_w / 2, height / 2 - 56 + i * 52};
}

void UIRenderer::Render(HDC hdc, int width, int height, const SamuraiSudoku& game, UIState& ui) {
    if (width <= 0 || height <= 0) return;
    UpdateLayout(width, height, ui);
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, width, height);
    HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);

    bool senior = ui.senior_mode;
    bool is_fa = (game.language == Language::Persian);

    COLORREF bg_clr = senior ? SENIOR_BG : MODERN_BG;
    COLORREF head_bg = senior ? SENIOR_HEADER_BG : MODERN_HEADER_BG;
    COLORREF head_txt = senior ? SENIOR_HEADER_TEXT : MODERN_HEADER_TEXT;

    HBRUSH bgBrush = CreateSolidBrush(bg_clr);
    RECT fullScreenRect = {0, 0, width, height};
    FillRect(memDC, &fullScreenRect, bgBrush);
    DeleteObject(bgBrush);

    const char* font_family = is_fa ? "Tahoma" : "Segoe UI";

    HFONT titleFont = CreateFontA(senior ? 32 : 30, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                  OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_family);
    HFONT buttonFont = CreateFontA(senior ? 20 : 16, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                   OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_family);
    HFONT infoFont = CreateFontA(senior ? 20 : 16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                 OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, font_family);

    // -------------------------------------------------------------
    // SCREEN 1: MAIN MENU
    // -------------------------------------------------------------
    if (ui.screen == AppScreen::MainMenu) {
        SelectObject(memDC, titleFont);
        SetTextColor(memDC, senior ? RGB(15, 23, 42) : RGB(30, 40, 60));
        SetBkMode(memDC, TRANSPARENT);

        DrawBrandMark(memDC, width / 2, 22);
        std::wstring main_title = is_fa ? L"سودوکو سامورایی" : L"SAMURAI SUDOKU";
        RECT titleRect = {24, 70, width - 24, 116};
        DrawTextW_Centered(memDC, titleRect, main_title, senior ? RGB(15, 23, 42) : RGB(20, 31, 51), titleFont);
        RECT subtitleRect = {24, 116, width - 24, 146};
        DrawTextW_Centered(memDC, subtitleRect,
            is_fa ? L"پنج جدول به‌هم‌پیوسته؛ یک چالش آرام و آفلاین" : L"Five connected grids. One thoughtful offline challenge.",
            senior ? RGB(51, 65, 85) : RGB(81, 97, 117), infoFont);

        const RECT& r1 = ui.menu_buttons[0];
        const RECT& r2 = ui.menu_buttons[1];
        const RECT& r3 = ui.menu_buttons[2];
        const RECT& r4 = ui.menu_buttons[3];
        const RECT& r5 = ui.menu_buttons[4];
        const RECT& r6 = ui.menu_buttons[5];


        COLORREF btn_bg = senior ? RGB(224, 231, 255) : RGB(232, 238, 247);
        COLORREF btn_txt = RGB(20, 31, 51);

        DrawButtonW(memDC, r1, is_fa ? L"ادامه / شروع بازی" : L"Continue / Start", RGB(37, 99, 235), RGB(255, 255, 255), buttonFont);
        DrawButtonW(memDC, r2, is_fa ? L"۲. بازی جدید" : L"2. New Game", RGB(15, 118, 110), RGB(255, 255, 255), buttonFont);
        DrawButtonW(memDC, r3, is_fa ? (senior ? L"۳. پوسته: پرکنتراست" : L"۳. پوسته: مدرن") : (senior ? L"3. High contrast" : L"3. Modern theme"), btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r4, is_fa ? L"۴. تغییر زبان (فارسی / English)" : L"4. Language: English / فارسی", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r5, is_fa ? L"۵. راهنمای کلیدها" : L"5. Controls Guide", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r6, is_fa ? L"۶. خروج از برنامه" : L"6. Quit Game", btn_bg, btn_txt, buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 2: DIFFICULTY SELECTION
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::DifficultySelect) {
        std::wstring diff_title = is_fa ? L"انتخاب سطح سختی" : L"SELECT DIFFICULTY LEVEL";
        RECT titleRect = {24, 70, width - 24, 126};
        DrawTextW_Centered(memDC, titleRect, diff_title, senior ? RGB(15, 23, 42) : RGB(30, 40, 60), titleFont);

        const RECT& r1 = ui.menu_buttons[0];
        const RECT& r2 = ui.menu_buttons[1];
        const RECT& r3 = ui.menu_buttons[2];
        const RECT& r4 = ui.menu_buttons[3];
        const RECT& r5 = ui.menu_buttons[4];
        const RECT& r6 = ui.menu_buttons[5];


        COLORREF btn_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF btn_txt = RGB(15, 23, 42);

        DrawButtonW(memDC, r1, is_fa ? L"۱. آسان (Easy)" : L"1. Easy", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r2, is_fa ? L"۲. متوسط (Medium)" : L"2. Medium", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r3, is_fa ? L"۳. سخت (Hard)" : L"3. Hard", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r4, is_fa ? L"۴. حرفه‌ای (Expert)" : L"4. Expert", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r5, is_fa ? L"۵. استاد (Master)" : L"5. Master", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r6, is_fa ? L"بازگشت به منوی اصلی >" : L"< Back to Main Menu", RGB(254, 226, 226), RGB(153, 27, 27), buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 3: CONTROLS GUIDE
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::ControlsGuide) {
        std::wstring guide_title = is_fa ? L"راهنمای بازی و کلیدها" : L"CONTROLS & GUIDE";
        RECT titleRect = {0, height / 10, width, height / 6};
        DrawTextW_Centered(memDC, titleRect, guide_title, senior ? RGB(15, 23, 42) : RGB(30, 40, 60), titleFont);

        int y = 140;
        int step = (height - 230) / 9;

        std::vector<std::wstring> lines;
        if (is_fa) {
            lines = {
                L"ماوس: انتخاب خانه؛ کلیک دوباره یا راست‌کلیک: انتخاب عدد",
                L"کلیدهای جهت‌نما: حرکت خانه انتخابی روی ۵ جدول سودوکو",
                L"کیبورد ۱ تا ۹: ورود عدد در خانه انتخابی",
                L"کلید Delete یا Backspace: پاک کردن خانه انتخابی",
                L"کلید Shift + اعداد یا کلید N: فعال/غیرفعال‌سازی حالت یادداشت‌گذاری",
                L"کلید Ctrl + Z: بازگشت به حرکت قبلی (Undo)",
                L"کلید ESC: باز کردن منوی بازی",
                L"کلید C یا دکمه بررسی: نمایش خطاها در حالت بررسی دستی",
                L"حالت پرکنتراست برای خوانایی بهتر و فونت‌های درشت‌تر"
            };
        } else {
            lines = {
                L"Mouse: Select a cell; click again or right-click for the keypad.",
                L"Arrow Keys: Navigate selected cell across the 5 overlapping grids.",
                L"Numbers 1-9: Enter digit into selected cell.",
                L"Delete / Backspace: Clear selected cell.",
                L"Shift + 1-9 or 'N' Key: Toggle Pencil Notes mode.",
                L"Ctrl + Z: Undo previous move.",
                L"ESC: Open Game Menu.",
                L"C / Check: Reveal errors. Manual checking hides feedback until requested.",
                L"High-Contrast Theme: Larger text and clearer visual separation."
            };
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            RECT lRect = {32, y + (int)i * step, width - 32, y + ((int)i + 1) * step - 6};
            SelectObject(memDC, infoFont);
            SetTextColor(memDC, RGB(30, 40, 60));
            DrawTextW(memDC, lines[i].c_str(), -1, &lRect, DT_CENTER | DT_WORDBREAK | DT_NOPREFIX);
        }

        RECT r_back = ui.menu_buttons[0];
        DrawButtonW(memDC, r_back, is_fa ? L"بازگشت به منوی اصلی" : L"< Back to Main Menu", RGB(224, 231, 255), RGB(15, 23, 42), buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 4: IN-GAME BOARD & RESPONSIVE LAYOUT
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::InGame) {
        const int header_height = 76;
        RECT headerRect = {0, 0, width, header_height};
        HBRUSH headerBrush = CreateSolidBrush(head_bg);
        FillRect(memDC, &headerRect, headerBrush);
        DeleteObject(headerBrush);
        RECT brandRect = {24, 8, width - 24, 38};
        DrawTextW_Centered(memDC, brandRect, is_fa ? L"سودوکو سامورایی" : L"SAMURAI SUDOKU", head_txt, buttonFont);
        const std::wstring en[] = {L"Easy", L"Medium", L"Hard", L"Expert", L"Master"};
        const std::wstring fa[] = {L"آسان", L"متوسط", L"سخت", L"حرفه‌ای", L"استاد"};
        const int difficulty = (std::max)(0, (std::min)(4, static_cast<int>(game.difficulty)));
        wchar_t timer[40];
        swprintf(timer, 40, L"%02d:%02d", game.elapsed_seconds / 60, game.elapsed_seconds % 60);
        std::wstring status = (is_fa ? fa[difficulty] : en[difficulty]) + L"   ·   " + timer + L"   ·   ";
        if (game.manual_check && !game.check_revealed)
            status += is_fa ? L"بررسی نشده" : L"Not checked";
        else if (game.is_completed())
            status += is_fa ? L"پازل کامل شد!" : L"Puzzle complete!";
        else
            status += (is_fa ? L"خطاها: " : L"Errors: ") + std::to_wstring(game.error_count);
        RECT statusRect = {24, 40, width - 24, 70};
        DrawTextW_Centered(memDC, statusRect, status, head_txt, infoFont);
        const int cell_sz = ui.cell_size;
        const int origin_x = ui.grid_origin_x;
        const int origin_y = ui.grid_origin_y;
        COLORREF action_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF active_notes_bg = RGB(254, 240, 138);

        DrawButtonW(memDC, ui.btn_undo, is_fa ? L"بازگردانی" : L"Undo", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_erase, is_fa ? L"پاک‌کن" : L"Erase Cell", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_notes, is_fa ? (ui.note_mode ? L"یادداشت [روشن]" : L"یادداشت [خاموش]") : (ui.note_mode ? L"Notes [ON]" : L"Notes [OFF]"), ui.note_mode ? active_notes_bg : action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_reset, is_fa ? L"ریست بازی" : L"Reset Game", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_theme, is_fa ? (senior ? L"پوسته پرکنتراست" : L"پوسته مدرن") : (senior ? L"High Contrast" : L"Modern Theme"), action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_lang, is_fa ? L"زبان: فارسی" : L"Lang: English", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_menu, is_fa ? L"منوی بازی (ESC)" : L"Pause (ESC)", action_bg, RGB(15, 23, 42), buttonFont);

        DrawButtonW(memDC, ui.btn_check, is_fa ? L"بررسی (C)" : L"Check (C)", RGB(15, 118, 110), RGB(255, 255, 255), buttonFont);
        DrawButtonW(memDC, ui.btn_check_mode,
            is_fa ? (game.manual_check ? L"بررسی: دستی" : L"بررسی: خودکار") : (game.manual_check ? L"Check: manual" : L"Check: automatic"),
            action_bg, RGB(15, 23, 42), buttonFont);
        int selected_val = 0;
        if (ui.selected_r >= 0 && ui.selected_c >= 0) {
            selected_val = game.board[ui.selected_r][ui.selected_c].value;
        }

        HFONT cellNumFont = CreateFontA(-(std::max)(10, cell_sz * (senior ? 70 : 62) / 100), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        HFONT noteFont = CreateFontA(-(std::max)(6, cell_sz * 26 / 100), 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                     OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

        COLORREF c_cell_bg = senior ? SENIOR_CELL_BG : MODERN_CELL_BG;
        COLORREF c_given_bg = senior ? SENIOR_GIVEN_BG : MODERN_GIVEN_BG;
        COLORREF c_overlap_bg = senior ? SENIOR_OVERLAP_BG : MODERN_OVERLAP_BG;
        COLORREF c_selected_bg = senior ? SENIOR_SELECTED_BG : MODERN_SELECTED_BG;
        COLORREF c_same_num_bg = senior ? SENIOR_SAME_NUM_BG : MODERN_SAME_NUM_BG;
        COLORREF c_error_bg = senior ? SENIOR_ERROR_BG : MODERN_ERROR_BG;

        COLORREF c_txt_given = senior ? SENIOR_TEXT_GIVEN : MODERN_TEXT_GIVEN;
        COLORREF c_txt_user = senior ? SENIOR_TEXT_USER : MODERN_TEXT_USER;
        COLORREF c_txt_error = senior ? SENIOR_TEXT_ERROR : MODERN_TEXT_ERROR;

        // Draw Cells
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

                HPEN thinPen = CreatePen(PS_SOLID, 1, RGB(194, 205, 217));
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

                    wchar_t val_str[4];
                    swprintf(val_str, 4, L"%d", cell.value);
                    DrawTextW_Centered(memDC, cRect, val_str, cell.is_error ? c_txt_error : (cell.is_given ? c_txt_given : c_txt_user), cellNumFont);
                } else if (cell.notes != 0) {
                    SelectObject(memDC, noteFont);
                    SetTextColor(memDC, senior ? SENIOR_TEXT_NOTE : MODERN_TEXT_NOTE);

                    int n_sz = cell_sz / 3;
                    for (int n = 1; n <= 9; ++n) {
                        if (cell.notes & (1 << n)) {
                            int nr = (n - 1) / 3;
                            int nc = (n - 1) % 3;
                            wchar_t n_str[2] = { (wchar_t)(L'0' + n), L'\0' };
                            RECT nRect = {cell_left + nc * n_sz, cell_top + nr * n_sz, cell_left + (nc + 1) * n_sz, cell_top + (nr + 1) * n_sz};
                            DrawTextW_Centered(memDC, nRect, n_str, senior ? SENIOR_TEXT_NOTE : MODERN_TEXT_NOTE, noteFont);
                        }
                    }
                }
            }
        }

        // Draw Subgrid Thick Borders
        HPEN thickPen = CreatePen(PS_SOLID, cell_sz < 22 ? 2 : (senior ? 4 : 3), RGB(15, 23, 42));
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

        // Standard Compact Pop-up Keypad (Proportional to cell size)
        if (ui.show_number_popup && ui.popup_r >= 0 && ui.popup_c >= 0) {
            int pop_w = cell_sz * 4;
            if (pop_w < 130) pop_w = 130;
            if (pop_w > 180) pop_w = 180;
            pop_w -= pop_w % 3;
            int header_h = 32;
            int pop_h = pop_w + header_h;

            int pop_x = origin_x + ui.popup_c * cell_sz + cell_sz / 2;
            int pop_y = origin_y + ui.popup_r * cell_sz + cell_sz / 2;

            if (pop_x + pop_w > width - 10) pop_x = width - pop_w - 10;
            if (pop_y + pop_h > height - 10) pop_y = height - pop_h - 10;
            if (pop_x < 10) pop_x = 10;
            if (pop_y < 10) pop_y = 10;

            RECT pRect = {pop_x, pop_y, pop_x + pop_w, pop_y + pop_h};
            ui.popup_rect = pRect;

            // Keypad Shadow & Solid White Background
            HBRUSH popBg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &pRect, popBg);
            DeleteObject(popBg);
            FrameRect(memDC, &pRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

            // Header of Keypad
            RECT popHeader = {pop_x, pop_y, pop_x + pop_w, pop_y + header_h};
            HBRUSH pHeadBg = CreateSolidBrush(RGB(226, 232, 240));
            FillRect(memDC, &popHeader, pHeadBg);
            DeleteObject(pHeadBg);

            RECT r_erase_hdr = {pop_x, pop_y, pop_x + pop_w / 2, pop_y + header_h};
            RECT r_close_hdr = {pop_x + pop_w / 2, pop_y, pop_x + pop_w, pop_y + header_h};
            DrawTextW_Centered(memDC, r_erase_hdr, is_fa ? L"پاک" : L"Erase", RGB(15, 23, 42), infoFont);
            DrawTextW_Centered(memDC, r_close_hdr, is_fa ? L"بستن" : L"Close", RGB(15, 23, 42), infoFont);

            int btn_w_p = pop_w / 3;
            int btn_h_p = (pop_h - header_h) / 3;

            for (int r = 0; r < 3; ++r) {
                for (int c = 0; c < 3; ++c) {
                    int num = r * 3 + c + 1;
                    int bx = pop_x + c * btn_w_p;
                    int by = pop_y + header_h + r * btn_h_p;
                    RECT bRect = {bx, by, bx + btn_w_p, by + btn_h_p};

                    FrameRect(memDC, &bRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

                    wchar_t num_str[2] = {(wchar_t)(L'0' + num), L'\0'};
                    DrawTextW_Centered(memDC, bRect, num_str, RGB(15, 23, 42), buttonFont);
                }
            }
        }

        // ESC Menu Overlay
        if (ui.show_esc_menu) {
            FillRect(memDC, &fullScreenRect, (HBRUSH)GetStockObject(LTGRAY_BRUSH));
            RECT menuRect = {width / 2 - 240, height / 2 - 180, width / 2 + 240, height / 2 + 180};
            HBRUSH menuBg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &menuRect, menuBg);
            DeleteObject(menuBg);
            RECT r_mtitle = {menuRect.left + 16, menuRect.top + 16, menuRect.right - 16, menuRect.top + 64};
            DrawTextW_Centered(memDC, r_mtitle, is_fa ? L"بازی متوقف است" : L"Game paused", RGB(15, 23, 42), titleFont);

            std::vector<std::wstring> mitems;
            if (is_fa) {
                mitems = {
                    L"۱. ادامه‌ی بازی",
                    L"۲. شروع مجدد همین بازی",
                    L"۳. بازی جدید",
                    L"۴. تغییر پوسته (پرکنتراست / مدرن)",
                    L"۵. خروج به منوی اصلی"
                };
            } else {
                mitems = {
                    L"[1] Resume Game",
                    L"[2] Restart Current Game",
                    L"[3] New Game",
                    L"[4] Toggle Contrast/Modern Theme",
                    L"[5] Main Menu"
                };
            }

            for (size_t i = 0; i < mitems.size(); ++i) {
                DrawButtonW(memDC, ui.pause_buttons[i], mitems[i], RGB(235, 240, 248), RGB(15, 23, 42), buttonFont);
            }
        }

        SelectObject(memDC, GetStockObject(SYSTEM_FONT));
        DeleteObject(cellNumFont);
        DeleteObject(noteFont);
    }

    // Loading Overlay
    if (ui.is_loading) {
        FillRect(memDC, &fullScreenRect, (HBRUSH)GetStockObject(GRAY_BRUSH));

        RECT loadRect = {width / 2 - 220, height / 2 - 60, width / 2 + 220, height / 2 + 60};
        HBRUSH boxBg = CreateSolidBrush(RGB(255, 255, 255));
        FillRect(memDC, &loadRect, boxBg);
        DeleteObject(boxBg);
        FrameRect(memDC, &loadRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

        std::wstring load_msg = is_fa ? L"در حال ساخت پازل سودوکو سامورایی..." : L"Generating Samurai Sudoku Puzzle...";
        DrawTextW_Centered(memDC, loadRect, load_msg, RGB(15, 23, 42), buttonFont);
    }

    SelectObject(memDC, GetStockObject(SYSTEM_FONT));
    DeleteObject(titleFont);
    DeleteObject(buttonFont);
    DeleteObject(infoFont);

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
