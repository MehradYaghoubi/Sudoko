#include "../src/ui_renderer.h"
#include <cassert>
#include <iostream>
#include <fstream>
#include <vector>

static bool overlaps(RECT a, RECT b) {
    return a.left < b.right && b.left < a.right && a.top < b.bottom && b.top < a.bottom;
}
static void snapshot(const char* filename, int width, int height, SamuraiSudoku& game, UIState& ui) {
    HDC screen = GetDC(nullptr);
    HDC dc = CreateCompatibleDC(screen);
    HBITMAP bitmap = CreateCompatibleBitmap(screen, width, height);
    HGDIOBJ previous = SelectObject(dc, bitmap);
    UIRenderer::Render(dc, width, height, game, ui);
    SelectObject(dc, previous);
    BITMAPINFO info = {};
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -height;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    info.bmiHeader.biCompression = BI_RGB;
    std::vector<char> pixels(width * height * 4);
    assert(GetDIBits(dc, bitmap, 0, height, pixels.data(), &info, DIB_RGB_COLORS));
    BITMAPFILEHEADER header = {};
    header.bfType = 0x4d42;
    header.bfOffBits = sizeof(header) + sizeof(info.bmiHeader);
    header.bfSize = header.bfOffBits + static_cast<DWORD>(pixels.size());
    std::ofstream file(filename, std::ios::binary);
    file.write(reinterpret_cast<char*>(&header), sizeof(header));
    file.write(reinterpret_cast<char*>(&info.bmiHeader), sizeof(info.bmiHeader));
    file.write(pixels.data(), pixels.size());
    DeleteObject(bitmap);
    DeleteDC(dc);
    ReleaseDC(nullptr, screen);
}

int main(int argc, char**) {
    for (int w : {560, 640, 800, 959, 960, 1280, 1920}) {
        for (int h : {600, 720, 900, 1080}) {
            for (bool senior : {false, true}) {
                UIState ui;
                ui.senior_mode = senior;
                UIRenderer::UpdateLayout(w, h, ui);
                RECT buttons[] = {ui.btn_check, ui.btn_check_mode, ui.btn_notes, ui.btn_undo,
                    ui.btn_erase, ui.btn_reset, ui.btn_theme, ui.btn_lang, ui.btn_menu};
                assert(ui.grid_rect.left >= 0 && ui.grid_rect.top >= 76);
                assert(ui.grid_rect.right < w && ui.grid_rect.bottom < h);
                for (int i = 0; i < 9; ++i) {
                    RECT r = buttons[i];
                    assert(r.left >= 0 && r.top >= 76 && r.right < w && r.bottom < h);
                    assert(!overlaps(r, ui.grid_rect));
                    for (int j = 0; j < i; ++j) assert(!overlaps(r, buttons[j]));
                }
                for (RECT r : ui.menu_buttons) assert(r.top >= 150 && r.bottom <= h - 16);
                int row, col;
                UIRenderer::GetCellFromPos(ui.grid_origin_x + 1, ui.grid_origin_y + 1, ui, row, col);
                assert(row == 0 && col == 0);
                UIRenderer::GetCellFromPos(ui.grid_rect.right, ui.grid_rect.bottom, ui, row, col);
                assert(row == -1 && col == -1);
            }
        }
    }
    if (argc > 1) {
        SamuraiSudoku game;
        bool senior = true;
        std::ifstream save("save.json");
        std::string data((std::istreambuf_iterator<char>(save)), std::istreambuf_iterator<char>());
        assert(game.deserialize_from_json(data, senior));
        UIState ui;
        ui.screen = AppScreen::InGame;
        ui.senior_mode = false;
        game.language = Language::English;
        snapshot(".qa-preview/wide-en.bmp", 1280, 800, game, ui);
        game.language = Language::Persian;
        ui.senior_mode = true;
        snapshot(".qa-preview/compact-fa.bmp", 560, 600, game, ui);
        ui.show_esc_menu = true;
        snapshot(".qa-preview/pause-fa.bmp", 560, 600, game, ui);
        ui.show_esc_menu = false;
        ui.screen = AppScreen::MainMenu;
        snapshot(".qa-preview/menu-fa.bmp", 560, 600, game, ui);
        ui.screen = AppScreen::ControlsGuide;
        snapshot(".qa-preview/guide-fa.bmp", 560, 600, game, ui);
    }
    std::cout << "Layout containment, non-overlap and hit tests passed (56 layouts).\n";
}
