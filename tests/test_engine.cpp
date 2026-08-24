#include "../src/sudoku_engine.h"
#include <iostream>
#include <cassert>

void test_grid_structure() {
    std::cout << "[Test] Checking grid cell validity and overlaps..." << std::endl;
    // Top-Left (0,0) is valid
    assert(SamuraiSudoku::is_valid_cell(0, 0));
    // Center (10,10) is valid
    assert(SamuraiSudoku::is_valid_cell(10, 10));
    // Blank spot (0,10) is invalid
    assert(!SamuraiSudoku::is_valid_cell(0, 10));

    // Overlapping area (6,6) belongs to Top-Left (0) and Center (2)
    int grids[2];
    int count = 0;
    SamuraiSudoku::get_grid_memberships(6, 6, grids, count);
    assert(count == 2);
    assert(grids[0] == 0 && grids[1] == 2);
    std::cout << "[Test] Grid structure test passed!" << std::endl;
}

void test_puzzle_generator_and_solver() {
    std::cout << "[Test] Generating puzzle..." << std::endl;
    SamuraiSudoku game;
    bool gen_ok = game.generate_puzzle(Difficulty::Easy);
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

    std::cout << "Givens: " << given_cells << ", Empty: " << empty_cells << std::endl;
    assert(given_cells > 100);
    assert(empty_cells > 0);
    assert(given_cells + empty_cells == 369);
    std::cout << "[Test] Puzzle generator test passed!" << std::endl;
}

void test_moves_and_undo() {
    std::cout << "[Test] Testing move, notes, error checking and undo..." << std::endl;
    SamuraiSudoku game;
    game.generate_puzzle(Difficulty::Easy);

    // Find first empty non-given cell
    int target_r = -1, target_c = -1;
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (SamuraiSudoku::is_valid_cell(r, c) && !game.board[r][c].is_given) {
                target_r = r;
                target_c = c;
                break;
            }
        }
        if (target_r != -1) break;
    }

    assert(target_r != -1);

    // Toggle note
    game.toggle_note(target_r, target_c, 5);
    assert((game.board[target_r][target_c].notes & (1 << 5)) != 0);

    // Set value
    game.set_cell_value(target_r, target_c, 5);
    assert(game.board[target_r][target_c].value == 5);
    assert(game.board[target_r][target_c].notes == 0); // Notes cleared on set value

    // Undo set value
    game.undo();
    assert(game.board[target_r][target_c].value == 0);
    assert((game.board[target_r][target_c].notes & (1 << 5)) != 0); // Note restored

    // Undo note toggle
    game.undo();
    assert(game.board[target_r][target_c].notes == 0);

    std::cout << "[Test] Moves and undo test passed!" << std::endl;
}

void test_serialization() {
    std::cout << "[Test] Testing JSON serialization and deserialization..." << std::endl;
    SamuraiSudoku game;
    game.generate_puzzle(Difficulty::Medium);
    game.elapsed_seconds = 120;

    std::string json_data = game.serialize_to_json();
    assert(!json_data.empty());

    SamuraiSudoku loaded_game;
    bool load_ok = loaded_game.deserialize_from_json(json_data);
    assert(load_ok);
    assert(loaded_game.difficulty == Difficulty::Medium);
    assert(loaded_game.elapsed_seconds == 120);

    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (SamuraiSudoku::is_valid_cell(r, c)) {
                assert(loaded_game.board[r][c].value == game.board[r][c].value);
                assert(loaded_game.board[r][c].solution == game.board[r][c].solution);
                assert(loaded_game.board[r][c].is_given == game.board[r][c].is_given);
                assert(loaded_game.board[r][c].notes == game.board[r][c].notes);
            }
        }
    }

    std::cout << "[Test] JSON serialization test passed!" << std::endl;
}

int main() {
    test_grid_structure();
    test_puzzle_generator_and_solver();
    test_moves_and_undo();
    test_serialization();
    std::cout << "ALL ENGINE TESTS PASSED SUCCESSFULLY!" << std::endl;
    return 0;
}
