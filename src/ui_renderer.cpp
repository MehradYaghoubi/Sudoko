#include "ui_renderer.h"
#include <string>
#include <cstdio>
#include <vector>

// Theme Color Definitions
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

// Senior / High-Contrast Theme
const COLORREF SENIOR_BG = RGB(255, 255, 255);
const COLORREF SENIOR_HEADER_BG = RGB(15, 23, 42);
const COLORREF SENIOR_HEADER_TEXT = RGB(255, 255, 255);
const COLORREF SENIOR_CELL_BG = RGB(255, 255, 255);
const COLORREF SENIOR_GIVEN_BG = RGB(226, 232, 240);
const COLORREF SENIOR_OVERLAP_BG = RGB(254, 240, 138);
const COLORREF SENIOR_SELECTED_BG = RGB(147, 197, 253);
const COLORREF SENIOR_SAME_NUM_BG = RGB(191, 219, 254);
const COLORREF SENIOR_ERROR_BG = RGB(254, 202, 202);

const COLORREF SENIOR_TEXT_GIVEN = RGB(0, 0, 0);
const COLORREF SENIOR_TEXT_USER = RGB(2, 132, 199);
const COLORREF SENIOR_TEXT_ERROR = RGB(185, 28, 28);
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
    if (mouse_x < ui.popup_rect.left || mouse_x > ui.popup_rect.right ||
        mouse_y < ui.popup_rect.top || mouse_y > ui.popup_rect.bottom) {
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
    SelectObject(hdc, font);
    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);
    DrawTextW(hdc, wtext.c_str(), (int)wtext.size(), &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

static void DrawButtonW(HDC hdc, RECT rect, const std::wstring& text, COLORREF bg, COLORREF textColor, HFONT font) {
    HBRUSH brush = CreateSolidBrush(bg);
    FillRect(hdc, &rect, brush);
    DeleteObject(brush);

    FrameRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));

    DrawTextW_Centered(hdc, rect, text, textColor, font);
}

void UIRenderer::Render(HDC hdc, int width, int height, const SamuraiSudoku& game, const UIState& ui) {
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

    HFONT titleFont = CreateFontA(senior ? 30 : 24, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
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

        std::wstring main_title = is_fa ? L"سودوکو سامورایی" : L"SAMURAI SUDOKU";
        RECT titleRect = {0, height / 10, width, height / 5};
        DrawTextW_Centered(memDC, titleRect, main_title, senior ? RGB(15, 23, 42) : RGB(30, 40, 60), titleFont);

        int btn_w = senior ? 360 : 300;
        int btn_h = senior ? 55 : 46;
        int start_y = height / 4 + 20;
        int gap = senior ? 18 : 14;

        RECT r1 = {width / 2 - btn_w / 2, start_y, width / 2 + btn_w / 2, start_y + btn_h};
        RECT r2 = {width / 2 - btn_w / 2, start_y + (btn_h + gap), width / 2 + btn_w / 2, start_y + (btn_h + gap) + btn_h};
        RECT r3 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 2, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 2 + btn_h};
        RECT r4 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 3, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 3 + btn_h};
        RECT r5 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 4, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 4 + btn_h};
        RECT r6 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 5, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 5 + btn_h};

        COLORREF btn_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF btn_txt = RGB(15, 23, 42);

        DrawButtonW(memDC, r1, is_fa ? L"۱. ادامه‌ی بازی" : L"1. Continue Game", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r2, is_fa ? L"۲. بازی جدید" : L"2. New Game", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r3, is_fa ? (senior ? L"۳. پوسته: پرکنتراست / مسن" : L"۳. پوسته: مدرن") : (senior ? L"3. Theme: SENIOR (High-Contrast)" : L"3. Theme: MODERN"), btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r4, is_fa ? L"۴. تغییر زبان (فارسی / English)" : L"4. Change Language (EN / FA)", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r5, is_fa ? L"۵. راهنمای کلیدها" : L"5. Controls Guide", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r6, is_fa ? L"۶. خروج از برنامه" : L"6. Quit Game", btn_bg, btn_txt, buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 2: DIFFICULTY SELECTION
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::DifficultySelect) {
        std::wstring diff_title = is_fa ? L"انتخاب سطح سختی" : L"SELECT DIFFICULTY LEVEL";
        RECT titleRect = {0, height / 8, width, height / 4};
        DrawTextW_Centered(memDC, titleRect, diff_title, senior ? RGB(15, 23, 42) : RGB(30, 40, 60), titleFont);

        int btn_w = senior ? 360 : 300;
        int btn_h = senior ? 52 : 44;
        int start_y = height / 3 - 20;
        int gap = senior ? 16 : 12;

        RECT r1 = {width / 2 - btn_w / 2, start_y, width / 2 + btn_w / 2, start_y + btn_h};
        RECT r2 = {width / 2 - btn_w / 2, start_y + (btn_h + gap), width / 2 + btn_w / 2, start_y + (btn_h + gap) + btn_h};
        RECT r3 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 2, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 2 + btn_h};
        RECT r4 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 3, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 3 + btn_h};
        RECT r5 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 4, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 4 + btn_h};
        RECT r6 = {width / 2 - btn_w / 2, start_y + (btn_h + gap) * 5, width / 2 + btn_w / 2, start_y + (btn_h + gap) * 5 + btn_h};

        COLORREF btn_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF btn_txt = RGB(15, 23, 42);

        DrawButtonW(memDC, r1, is_fa ? L"۱. آسان (Easy)" : L"1. Easy", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r2, is_fa ? L"۲. متوسط (Medium)" : L"2. Medium", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r3, is_fa ? L"۳. سخت (Hard)" : L"3. Hard", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r4, is_fa ? L"۴. حرفه‌ای (Expert)" : L"4. Expert", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r5, is_fa ? L"۵. غول (Master)" : L"5. Master (Monster)", btn_bg, btn_txt, buttonFont);
        DrawButtonW(memDC, r6, is_fa ? L"بازگشت به منوی اصلی >" : L"< Back to Main Menu", RGB(254, 226, 226), RGB(153, 27, 27), buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 3: CONTROLS GUIDE
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::ControlsGuide) {
        std::wstring guide_title = is_fa ? L"راهنمای بازی و کلیدها" : L"CONTROLS & GUIDE";
        RECT titleRect = {0, height / 10, width, height / 6};
        DrawTextW_Centered(memDC, titleRect, guide_title, senior ? RGB(15, 23, 42) : RGB(30, 40, 60), titleFont);

        int y = height / 4;
        int step = senior ? 36 : 28;

        std::vector<std::wstring> lines;
        if (is_fa) {
            lines = {
                L"ماوس: کلیک روی خانه برای انتخاب / کلیک مجدد یا راست‌کلیک برای باز کردن صفحه انتخاب عدد",
                L"کلیدهای جهت‌نما: حرکت خانه انتخابی روی ۵ جدول سودوکو",
                L"کیبورد ۱ تا ۹: ورود عدد در خانه انتخابی",
                L"کلید Delete یا Backspace: پاک کردن خانه انتخابی",
                L"کلید Shift + اعداد یا کلید N: فعال/غیرفعال‌سازی حالت یادداشت‌گذاری",
                L"کلید Ctrl + Z: بازگشت به حرکت قبلی (Undo)",
                L"کلید ESC: باز کردن منوی بازی",
                L"حالت مسن/پرکنتراست: برای وضوح، خوانایی و فونت‌های درشت‌تر"
            };
        } else {
            lines = {
                L"Mouse: Click cell to select. Click again or Right-click to open Pop-up keypad.",
                L"Arrow Keys: Navigate selected cell across the 5 overlapping grids.",
                L"Numbers 1-9: Enter digit into selected cell.",
                L"Delete / Backspace: Clear selected cell.",
                L"Shift + 1-9 or 'N' Key: Toggle Pencil Notes mode.",
                L"Ctrl + Z: Undo previous move.",
                L"ESC: Open Game Menu.",
                L"High-Contrast Mode: Designed for optimal visibility and readability."
            };
        }

        for (size_t i = 0; i < lines.size(); ++i) {
            RECT lRect = {0, y + (int)i * step, width, y + ((int)i + 1) * step};
            DrawTextW_Centered(memDC, lRect, lines[i], senior ? RGB(15, 23, 42) : RGB(30, 40, 60), infoFont);
        }

        RECT r_back = {width / 2 - 150, y + (int)lines.size() * step + 20, width / 2 + 150, y + (int)lines.size() * step + 65};
        DrawButtonW(memDC, r_back, is_fa ? L"بازگشت به منوی اصلی" : L"< Back to Main Menu", RGB(224, 231, 255), RGB(15, 23, 42), buttonFont);
    }
    // -------------------------------------------------------------
    // SCREEN 4: IN-GAME BOARD & RESPONSIVE LAYOUT
    // -------------------------------------------------------------
    else if (ui.screen == AppScreen::InGame) {
        // Guaranteed Fixed Header Bar Height
        int header_height = senior ? 55 : 45;
        RECT headerRect = {0, 0, width, header_height};
        HBRUSH headerBrush = CreateSolidBrush(head_bg);
        FillRect(memDC, &headerRect, headerBrush);
        DeleteObject(headerBrush);

        SelectObject(memDC, infoFont);
        SetBkMode(memDC, TRANSPARENT);

        // Header Labels
        std::wstring app_name = is_fa ? L"سودوکو سامورایی" : L"SAMURAI SUDOKU";
        RECT r_app = {20, 0, 220, header_height};
        DrawTextW(memDC, app_name.c_str(), (int)app_name.size(), &r_app, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        const std::wstring diff_names_en[] = {L"Easy", L"Medium", L"Hard", L"Expert", L"Master"};
        const std::wstring diff_names_fa[] = {L"آسان", L"متوسط", L"سخت", L"حرفه‌ای", L"غول"};
        std::wstring dname = is_fa ? diff_names_fa[static_cast<int>(game.difficulty)] : diff_names_en[static_cast<int>(game.difficulty)];

        wchar_t diff_str[64];
        swprintf(diff_str, 64, is_fa ? L"سختی: %s" : L"Diff: %s", dname.c_str());
        RECT r_diff = {240, 0, 400, header_height};
        DrawTextW(memDC, diff_str, (int)wcslen(diff_str), &r_diff, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        int mins = game.elapsed_seconds / 60;
        int secs = game.elapsed_seconds % 60;
        wchar_t time_str[64];
        swprintf(time_str, 64, is_fa ? L"زمان: %02d:%02d" : L"Time: %02d:%02d", mins, secs);
        RECT r_time = {410, 0, 560, header_height};
        DrawTextW(memDC, time_str, (int)wcslen(time_str), &r_time, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        wchar_t err_str[64];
        swprintf(err_str, 64, is_fa ? L"خطاها: %d" : L"Errors: %d", game.error_count);
        RECT r_err = {570, 0, 700, header_height};
        DrawTextW(memDC, err_str, (int)wcslen(err_str), &r_err, DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        // Strictly Enforce Board Calculation below Header (origin_y >= header_height + 15)
        int min_header_margin = 15;
        int sidebar_width = senior ? 180 : 150;
        int gap_board_side = 20;

        int avail_w = width - sidebar_width - gap_board_side - 40;
        int avail_h = height - header_height - min_header_margin - 20;

        int grid_px = avail_w < avail_h ? avail_w : avail_h;
        int cell_sz = grid_px / 21;
        if (cell_sz < 12) cell_sz = 12;
        grid_px = cell_sz * 21;

        int origin_x = 20;
        int origin_y = header_height + min_header_margin + (avail_h - grid_px) / 2;
        if (origin_y < header_height + min_header_margin) origin_y = header_height + min_header_margin;

        UIState* mutable_ui = const_cast<UIState*>(&ui);
        mutable_ui->cell_size = cell_sz;
        mutable_ui->grid_origin_x = origin_x;
        mutable_ui->grid_origin_y = origin_y;

        // Position Action Sidebar Buttons safely to the Right of Grid
        int sidebar_x = origin_x + grid_px + gap_board_side;
        int btn_w = sidebar_width;
        int btn_h = senior ? 48 : 40;
        int btn_gap = senior ? 16 : 12;

        int start_btn_y = origin_y + (grid_px - (7 * btn_h + 6 * btn_gap)) / 2;
        if (start_btn_y < header_height + min_header_margin) start_btn_y = header_height + min_header_margin;

        mutable_ui->btn_undo  = {sidebar_x, start_btn_y, sidebar_x + btn_w, start_btn_y + btn_h};
        mutable_ui->btn_erase = {sidebar_x, start_btn_y + (btn_h + btn_gap), sidebar_x + btn_w, start_btn_y + (btn_h + btn_gap) + btn_h};
        mutable_ui->btn_notes = {sidebar_x, start_btn_y + (btn_h + btn_gap) * 2, sidebar_x + btn_w, start_btn_y + (btn_h + btn_gap) * 2 + btn_h};
        mutable_ui->btn_reset = {sidebar_x, start_btn_y + (btn_h + btn_gap) * 3, sidebar_x + btn_w, start_btn_y + (btn_h + btn_gap) * 3 + btn_h};
        mutable_ui->btn_theme = {sidebar_x, start_btn_y + (btn_h + btn_gap) * 4, sidebar_x + btn_w, start_btn_y + (btn_h + btn_gap) * 4 + btn_h};
        mutable_ui->btn_lang  = {sidebar_x, start_btn_y + (btn_h + btn_gap) * 5, sidebar_x + btn_w, start_btn_y + (btn_h + btn_gap) * 5 + btn_h};
        mutable_ui->btn_menu  = {sidebar_x, start_btn_y + (btn_h + btn_gap) * 6, sidebar_x + btn_w, start_btn_y + (btn_h + btn_gap) * 6 + btn_h};

        COLORREF action_bg = senior ? RGB(224, 231, 255) : RGB(235, 240, 248);
        COLORREF active_notes_bg = RGB(254, 240, 138);

        DrawButtonW(memDC, ui.btn_undo, is_fa ? L"Undo (برگشت)" : L"Undo", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_erase, is_fa ? L"پاک‌کن" : L"Erase Cell", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_notes, is_fa ? (ui.note_mode ? L"یادداشت [روشن]" : L"یادداشت [خاموش]") : (ui.note_mode ? L"Notes [ON]" : L"Notes [OFF]"), ui.note_mode ? active_notes_bg : action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_reset, is_fa ? L"ریست بازی" : L"Reset Game", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_theme, is_fa ? (senior ? L"پوسته مسن" : L"پوسته مدرن") : (senior ? L"Senior UI" : L"Modern UI"), action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_lang, is_fa ? L"زبان: فارسی" : L"Lang: English", action_bg, RGB(15, 23, 42), buttonFont);
        DrawButtonW(memDC, ui.btn_menu, is_fa ? L"منوی اصلی (ESC)" : L"Main Menu (ESC)", action_bg, RGB(15, 23, 42), buttonFont);

        int selected_val = 0;
        if (ui.selected_r >= 0 && ui.selected_c >= 0) {
            selected_val = game.board[ui.selected_r][ui.selected_c].value;
        }

        HFONT cellNumFont = CreateFontA(cell_sz * (senior ? 70 : 55) / 100, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                                       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");
        HFONT noteFont = CreateFontA(cell_sz * 26 / 100, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
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

        // Standard Compact Pop-up Keypad (Proportional to cell size)
        if (ui.show_number_popup && ui.popup_r >= 0 && ui.popup_c >= 0) {
            int pop_w = cell_sz * 4;
            if (pop_w < 130) pop_w = 130;
            if (pop_w > 180) pop_w = 180;
            int header_h = 32;
            int pop_h = pop_w + header_h;

            int pop_x = origin_x + ui.popup_c * cell_sz + cell_sz / 2;
            int pop_y = origin_y + ui.popup_r * cell_sz + cell_sz / 2;

            if (pop_x + pop_w > width - 10) pop_x = width - pop_w - 10;
            if (pop_y + pop_h > height - 10) pop_y = height - pop_h - 10;
            if (pop_x < 10) pop_x = 10;
            if (pop_y < 10) pop_y = 10;

            RECT pRect = {pop_x, pop_y, pop_x + pop_w, pop_y + pop_h};
            mutable_ui->popup_rect = pRect;

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
            RECT menuRect = {width / 2 - 200, height / 2 - 180, width / 2 + 200, height / 2 + 180};
            HBRUSH menuBg = CreateSolidBrush(RGB(255, 255, 255));
            FillRect(memDC, &menuRect, menuBg);
            DeleteObject(menuBg);

            FrameRect(memDC, &menuRect, (HBRUSH)GetStockObject(BLACK_BRUSH));

            RECT r_mtitle = {width / 2 - 180, height / 2 - 160, width / 2 + 180, height / 2 - 110};
            DrawTextW_Centered(memDC, r_mtitle, is_fa ? L"منوی بازی" : L"GAME MENU", RGB(15, 23, 42), titleFont);

            std::vector<std::wstring> mitems;
            if (is_fa) {
                mitems = {
                    L"۱. ادامه‌ی بازی",
                    L"۲. شروع مجدد همین بازی",
                    L"۳. بازی جدید",
                    L"۴. تغییر پوسته (مسن / مدرن)",
                    L"۵. خروج به منوی اصلی"
                };
            } else {
                mitems = {
                    L"[1] Resume Game",
                    L"[2] Restart Current Game",
                    L"[3] New Game",
                    L"[4] Toggle Senior/Modern UI",
                    L"[5 / ESC] Main Menu"
                };
            }

            for (size_t i = 0; i < mitems.size(); ++i) {
                RECT r_item = {width / 2 - 170, height / 2 - 90 + (int)i * 45, width / 2 + 170, height / 2 - 50 + (int)i * 45};
                DrawTextW_Centered(memDC, r_item, mitems[i], RGB(15, 23, 42), buttonFont);
            }
        }

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

    DeleteObject(titleFont);
    DeleteObject(buttonFont);
    DeleteObject(infoFont);

    BitBlt(hdc, 0, 0, width, height, memDC, 0, 0, SRCCOPY);

    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
