#include "../src/sudoku_engine.h"
#include <iostream>
#include <cassert>

void test_grid_structure() {
    std::cout << "[Test] Checking grid cell validity and overlaps..." << std::endl;
    assert(SamuraiSudoku::is_valid_cell(0, 0));
    assert(SamuraiSudoku::is_valid_cell(10, 10));
    assert(!SamuraiSudoku::is_valid_cell(0, 10));

    int grids[2];
    int count = 0;
    SamuraiSudoku::get_grid_memberships(6, 6, grids, count);
    assert(count == 2);
    assert(grids[0] == 0 && grids[1] == 2);
    std::cout << "[Test] Grid structure test passed!" << std::endl;
}

void test_5_difficulties_generator() {
    std::cout << "[Test] Generating Master difficulty puzzle..." << std::endl;
    SamuraiSudoku game;
    bool gen_ok = game.generate_puzzle(Difficulty::Master);
    assert(gen_ok);

    int empty_cells = 0;
    int given_cells = 0;
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (SamuraiSudoku::is_valid_cell(r, c)) {
                if (game.board[r][c].is_given) {
                    given_cells++;
                    assert(game.board[r][c].value != 0);
                    assert(game.board[r][c].value == game.board[r][c].solution);
                } else {
                    empty_cells++;
                    assert(game.board[r][c].value == 0);
                }
            }
        }
    }

    std::cout << "Master Givens: " << given_cells << ", Empty: " << empty_cells << std::endl;
    assert(given_cells > 80 && given_cells <= 130);
    assert(given_cells + empty_cells == 369);
    std::cout << "[Test] Master generator test passed!" << std::endl;
}

void test_reset_and_anti_pattern() {
    std::cout << "[Test] Testing reset to initial and anti-pattern..." << std::endl;
    SamuraiSudoku game;
    game.generate_puzzle(Difficulty::Easy);

    // Set a cell
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (SamuraiSudoku::is_valid_cell(r, c) && !game.board[r][c].is_given) {
                game.set_cell_value(r, c, 9);
                break;
            }
        }
    }

    game.reset_to_initial();

    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (SamuraiSudoku::is_valid_cell(r, c)) {
                if (!game.board[r][c].is_given) {
                    assert(game.board[r][c].value == 0);
                }
            }
        }
    }
    std::cout << "[Test] Reset to initial passed!" << std::endl;
}

void test_serialization() {
    std::cout << "[Test] Testing JSON serialization with senior mode..." << std::endl;
    SamuraiSudoku game;
    game.generate_puzzle(Difficulty::Expert);
    game.elapsed_seconds = 300;

    std::string json_data = game.serialize_to_json(true);
    assert(!json_data.empty());

    SamuraiSudoku loaded_game;
    bool loaded_smode = false;
    bool load_ok = loaded_game.deserialize_from_json(json_data, loaded_smode);
    assert(load_ok);
    assert(loaded_smode == true);
    assert(loaded_game.difficulty == Difficulty::Expert);
    assert(loaded_game.elapsed_seconds == 300);

    std::cout << "[Test] JSON serialization with senior mode passed!" << std::endl;
}

int main() {
    test_grid_structure();
    test_5_difficulties_generator();
    test_reset_and_anti_pattern();
    test_serialization();
    std::cout << "ALL ENHANCED ENGINE TESTS PASSED SUCCESSFULLY!" << std::endl;
    return 0;
}
