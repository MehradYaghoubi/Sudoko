#ifndef SUDOKU_ENGINE_H
#define SUDOKU_ENGINE_H

#include <vector>
#include <cstdint>
#include <string>

enum class Difficulty {
    Easy = 0,
    Medium = 1,
    Hard = 2,
    Expert = 3,
    Master = 4
};

enum class Language {
    English = 0,
    Persian = 1
};

struct Position {
    int r;
    int c;
};

struct SubGridOffset {
    int r;
    int c;
};

const SubGridOffset GRID_OFFSETS[5] = {
    {0, 0},   // 0: Top-Left
    {0, 12},  // 1: Top-Right
    {6, 6},   // 2: Center
    {12, 0},  // 3: Bottom-Left
    {12, 12}  // 4: Bottom-Right
};

struct Cell {
    int value = 0;      // Current value (0 = empty, 1-9)
    int solution = 0;   // Correct solution value (1-9)
    bool is_given = false;
    uint16_t notes = 0; // Bitmask for notes 1-9 (1 << num)
    bool is_error = false;
};

struct MoveHistory {
    int r;
    int c;
    int prev_value;
    int new_value;
    uint16_t prev_notes;
    uint16_t new_notes;
};

class SamuraiSudoku {
public:
    Cell board[21][21];
    Difficulty difficulty = Difficulty::Medium;
    Language language = Language::Persian;
    int elapsed_seconds = 0;
    int error_count = 0;
    std::vector<MoveHistory> history;

    SamuraiSudoku();

    static bool is_valid_cell(int r, int c);
    static void get_grid_memberships(int r, int c, int grids[2], int& count);

    void reset();
    void reset_to_initial();
    bool is_valid_move(int r, int c, int val, const int grid[21][21]) const;
    bool is_completed() const;

    // Solver and generator
    bool solve(int grid[21][21], int& solution_count, int max_solutions = 2) const;
    bool generate_puzzle(Difficulty diff);
    void apply_anti_pattern_transformations();

    // Move actions
    bool set_cell_value(int r, int c, int val);
    bool toggle_note(int r, int c, int note_num);
    bool clear_cell(int r, int c);
    bool undo();

    // Verification
    void recheck_errors();

    // JSON Serialization
    std::string serialize_to_json(bool senior_mode = false) const;
    bool deserialize_from_json(const std::string& json_str, bool& out_senior_mode);
};

#endif // SUDOKU_ENGINE_H
