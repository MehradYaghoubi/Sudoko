#include "sudoku_engine.h"
#include <algorithm>
#include <random>
#include <chrono>
#include <sstream>
#include <iostream>
#include <numeric>

SamuraiSudoku::SamuraiSudoku() {
    reset();
}

bool SamuraiSudoku::is_valid_cell(int r, int c) {
    if (r < 0 || r >= 21 || c < 0 || c >= 21) return false;
    for (int g = 0; g < 5; ++g) {
        int gr = r - GRID_OFFSETS[g].r;
        int gc = c - GRID_OFFSETS[g].c;
        if (gr >= 0 && gr < 9 && gc >= 0 && gc < 9) {
            return true;
        }
    }
    return false;
}

void SamuraiSudoku::get_grid_memberships(int r, int c, int grids[2], int& count) {
    count = 0;
    for (int g = 0; g < 5; ++g) {
        int gr = r - GRID_OFFSETS[g].r;
        int gc = c - GRID_OFFSETS[g].c;
        if (gr >= 0 && gr < 9 && gc >= 0 && gc < 9) {
            grids[count++] = g;
        }
    }
}

void SamuraiSudoku::reset() {
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            board[r][c] = Cell();
        }
    }
    history.clear();
    elapsed_seconds = 0;
    error_count = 0;
}

void SamuraiSudoku::reset_to_initial() {
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c)) {
                if (!board[r][c].is_given) {
                    board[r][c].value = 0;
                    board[r][c].notes = 0;
                    board[r][c].is_error = false;
                }
            }
        }
    }
    history.clear();
    recheck_errors();
}

bool SamuraiSudoku::is_valid_move(int r, int c, int val, const int grid[21][21]) const {
    if (val == 0) return true;
    int grids[2];
    int g_count = 0;
    get_grid_memberships(r, c, grids, g_count);

    for (int idx = 0; idx < g_count; ++idx) {
        int g = grids[idx];
        int gr = r - GRID_OFFSETS[g].r;
        int gc = c - GRID_OFFSETS[g].c;
        int base_r = GRID_OFFSETS[g].r;
        int base_c = GRID_OFFSETS[g].c;

        for (int i = 0; i < 9; ++i) {
            int check_c = base_c + i;
            if (check_c != c && grid[r][check_c] == val) return false;
        }

        for (int i = 0; i < 9; ++i) {
            int check_r = base_r + i;
            if (check_r != r && grid[check_r][c] == val) return false;
        }

        int box_start_r = base_r + (gr / 3) * 3;
        int box_start_c = base_c + (gc / 3) * 3;
        for (int br = 0; br < 3; ++br) {
            for (int bc = 0; bc < 3; ++bc) {
                int curr_r = box_start_r + br;
                int curr_c = box_start_c + bc;
                if ((curr_r != r || curr_c != c) && grid[curr_r][curr_c] == val) {
                    return false;
                }
            }
        }
    }
    return true;
}

bool SamuraiSudoku::solve(int grid[21][21], int& solution_count, int max_solutions) const {
    int min_candidates = 10;
    int best_r = -1;
    int best_c = -1;
    std::vector<int> best_nums;

    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c) && grid[r][c] == 0) {
                std::vector<int> valid_nums;
                for (int val = 1; val <= 9; ++val) {
                    if (is_valid_move(r, c, val, grid)) {
                        valid_nums.push_back(val);
                    }
                }
                if (valid_nums.empty()) {
                    return false;
                }
                if ((int)valid_nums.size() < min_candidates) {
                    min_candidates = (int)valid_nums.size();
                    best_r = r;
                    best_c = c;
                    best_nums = valid_nums;
                    if (min_candidates == 1) break;
                }
            }
        }
        if (min_candidates == 1) break;
    }

    if (best_r == -1) {
        solution_count++;
        return solution_count >= max_solutions;
    }

    static thread_local std::mt19937 rng(std::random_device{}());
    if (max_solutions == 1) {
        std::shuffle(best_nums.begin(), best_nums.end(), rng);
    }

    for (int val : best_nums) {
        grid[best_r][best_c] = val;
        if (solve(grid, solution_count, max_solutions)) {
            return true;
        }
        grid[best_r][best_c] = 0;
    }

    return false;
}

void SamuraiSudoku::apply_anti_pattern_transformations() {
    std::random_device rd;
    std::mt19937 rng(rd());

    std::vector<int> p = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::shuffle(p.begin(), p.end(), rng);
    int map_digit[10] = {0};
    for (int i = 0; i < 9; ++i) map_digit[i + 1] = p[i];

    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c)) {
                if (board[r][c].value != 0) board[r][c].value = map_digit[board[r][c].value];
                if (board[r][c].solution != 0) board[r][c].solution = map_digit[board[r][c].solution];
            }
        }
    }

    int rot = (rng() % 4);
    if (rot > 0) {
        Cell new_board[21][21];
        for (int r = 0; r < 21; ++r) {
            for (int c = 0; c < 21; ++c) {
                int nr = r, nc = c;
                if (rot == 1) { nr = c; nc = 20 - r; }
                else if (rot == 2) { nr = 20 - r; nc = 20 - c; }
                else if (rot == 3) { nr = 20 - c; nc = r; }

                if (is_valid_cell(nr, nc)) {
                    new_board[nr][nc] = board[r][c];
                }
            }
        }
        std::copy(&new_board[0][0], &new_board[0][0] + 21 * 21, &board[0][0]);
    }
}

bool SamuraiSudoku::generate_puzzle(Difficulty diff) {
    reset();
    difficulty = diff;

    int temp_grid[21][21] = {0};

    std::random_device rd;
    std::mt19937 g(rd());

    int solution_count = 0;
    solve(temp_grid, solution_count, 1);

    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c)) {
                board[r][c].solution = temp_grid[r][c];
                board[r][c].value = temp_grid[r][c];
                board[r][c].is_given = true;
            }
        }
    }

    std::vector<Position> cells;
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c)) {
                cells.push_back({r, c});
            }
        }
    }
    std::shuffle(cells.begin(), cells.end(), g);

    int target_givens = 180;
    switch (diff) {
        case Difficulty::Easy:   target_givens = 220; break;
        case Difficulty::Medium: target_givens = 180; break;
        case Difficulty::Hard:   target_givens = 150; break;
        case Difficulty::Expert: target_givens = 130; break;
        case Difficulty::Master: target_givens = 115; break;
    }

    int current_givens = (int)cells.size();

    for (const auto& pos : cells) {
        if (current_givens <= target_givens) break;

        int backup = temp_grid[pos.r][pos.c];
        temp_grid[pos.r][pos.c] = 0;

        int check_grid[21][21];
        std::copy(&temp_grid[0][0], &temp_grid[0][0] + 21 * 21, &check_grid[0][0]);

        int count = 0;
        solve(check_grid, count, 2);

        if (count == 1) {
            board[pos.r][pos.c].value = 0;
            board[pos.r][pos.c].is_given = false;
            current_givens--;
        } else {
            temp_grid[pos.r][pos.c] = backup;
        }
    }

    apply_anti_pattern_transformations();

    return true;
}

bool SamuraiSudoku::set_cell_value(int r, int c, int val) {
    if (!is_valid_cell(r, c) || board[r][c].is_given) return false;

    if (board[r][c].value == val) return false;

    MoveHistory hist;
    hist.r = r;
    hist.c = c;
    hist.prev_value = board[r][c].value;
    hist.new_value = val;
    hist.prev_notes = board[r][c].notes;
    hist.new_notes = board[r][c].notes;

    board[r][c].value = val;
    if (val != 0) {
        board[r][c].notes = 0;
    }

    history.push_back(hist);
    recheck_errors();
    return true;
}

bool SamuraiSudoku::toggle_note(int r, int c, int note_num) {
    if (!is_valid_cell(r, c) || board[r][c].is_given || board[r][c].value != 0) return false;
    if (note_num < 1 || note_num > 9) return false;

    MoveHistory hist;
    hist.r = r;
    hist.c = c;
    hist.prev_value = board[r][c].value;
    hist.new_value = board[r][c].value;
    hist.prev_notes = board[r][c].notes;

    board[r][c].notes ^= (1 << note_num);
    hist.new_notes = board[r][c].notes;

    history.push_back(hist);
    return true;
}

bool SamuraiSudoku::clear_cell(int r, int c) {
    if (!is_valid_cell(r, c) || board[r][c].is_given) return false;

    if (board[r][c].value == 0 && board[r][c].notes == 0) return false;

    MoveHistory hist;
    hist.r = r;
    hist.c = c;
    hist.prev_value = board[r][c].value;
    hist.new_value = 0;
    hist.prev_notes = board[r][c].notes;
    hist.new_notes = 0;

    board[r][c].value = 0;
    board[r][c].notes = 0;

    history.push_back(hist);
    recheck_errors();
    return true;
}

bool SamuraiSudoku::undo() {
    if (history.empty()) return false;

    MoveHistory last = history.back();
    history.pop_back();

    board[last.r][last.c].value = last.prev_value;
    board[last.r][last.c].notes = last.prev_notes;

    recheck_errors();
    return true;
}

void SamuraiSudoku::recheck_errors() {
    int current_grid[21][21];
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            current_grid[r][c] = board[r][c].value;
            board[r][c].is_error = false;
        }
    }

    int errors = 0;
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c) && board[r][c].value != 0 && !board[r][c].is_given) {
                int val = board[r][c].value;
                if (!is_valid_move(r, c, val, current_grid) || val != board[r][c].solution) {
                    board[r][c].is_error = true;
                    errors++;
                }
            }
        }
    }
    error_count = errors;
}

bool SamuraiSudoku::is_completed() const {
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c)) {
                if (board[r][c].value == 0 || board[r][c].value != board[r][c].solution) {
                    return false;
                }
            }
        }
    }
    return true;
}

std::string SamuraiSudoku::serialize_to_json(bool senior_mode) const {
    std::ostringstream ss;
    ss << "{\n";
    ss << "  \"difficulty\": " << static_cast<int>(difficulty) << ",\n";
    ss << "  \"language\": " << static_cast<int>(language) << ",\n";
    ss << "  \"elapsed_seconds\": " << elapsed_seconds << ",\n";
    ss << "  \"error_count\": " << error_count << ",\n";
    ss << "  \"senior_mode\": " << (senior_mode ? "true" : "false") << ",\n";
    ss << "  \"cells\": [\n";

    bool first = true;
    for (int r = 0; r < 21; ++r) {
        for (int c = 0; c < 21; ++c) {
            if (is_valid_cell(r, c)) {
                if (!first) ss << ",\n";
                first = false;
                ss << "    {\"r\":" << r << ",\"c\":" << c
                   << ",\"v\":" << board[r][c].value
                   << ",\"s\":" << board[r][c].solution
                   << ",\"g\":" << (board[r][c].is_given ? 1 : 0)
                   << ",\"n\":" << board[r][c].notes << "}";
            }
        }
    }
    ss << "\n  ]\n}";
    return ss.str();
}

bool SamuraiSudoku::deserialize_from_json(const std::string& json_str, bool& out_senior_mode) {
    reset();
    out_senior_mode = false;

    auto find_val = [&](const std::string& key) -> std::string {
        size_t pos = json_str.find("\"" + key + "\"");
        if (pos == std::string::npos) return "";
        size_t colon = json_str.find(":", pos);
        if (colon == std::string::npos) return "";
        size_t comma = json_str.find_first_of(",}\n", colon);
        if (comma == std::string::npos) return "";
        std::string val = json_str.substr(colon + 1, comma - colon - 1);
        val.erase(0, val.find_first_not_of(" \t\r\n"));
        val.erase(val.find_last_not_of(" \t\r\n") + 1);
        return val;
    };

    std::string diff_str = find_val("difficulty");
    if (!diff_str.empty()) difficulty = static_cast<Difficulty>(std::stoi(diff_str));

    std::string lang_str = find_val("language");
    if (!lang_str.empty()) language = static_cast<Language>(std::stoi(lang_str));

    std::string time_str = find_val("elapsed_seconds");
    if (!time_str.empty()) elapsed_seconds = std::stoi(time_str);

    std::string err_str = find_val("error_count");
    if (!err_str.empty()) error_count = std::stoi(err_str);

    std::string smode_str = find_val("senior_mode");
    if (!smode_str.empty()) out_senior_mode = (smode_str == "true");

    size_t cells_pos = json_str.find("\"cells\"");
    if (cells_pos == std::string::npos) return false;

    size_t pos = cells_pos;
    while ((pos = json_str.find("{", pos)) != std::string::npos) {
        size_t end_pos = json_str.find("}", pos);
        if (end_pos == std::string::npos) break;

        std::string cell_obj = json_str.substr(pos, end_pos - pos + 1);

        auto parse_cell_key = [&](const std::string& key) -> int {
            size_t kpos = cell_obj.find("\"" + key + "\"");
            if (kpos == std::string::npos) return 0;
            size_t colon = cell_obj.find(":", kpos);
            if (colon == std::string::npos) return 0;
            return std::stoi(cell_obj.substr(colon + 1));
        };

        int r = parse_cell_key("r");
        int c = parse_cell_key("c");
        if (is_valid_cell(r, c)) {
            board[r][c].value = parse_cell_key("v");
            board[r][c].solution = parse_cell_key("s");
            board[r][c].is_given = (parse_cell_key("g") == 1);
            board[r][c].notes = static_cast<uint16_t>(parse_cell_key("n"));
        }

        pos = end_pos + 1;
    }

    recheck_errors();
    return true;
}
