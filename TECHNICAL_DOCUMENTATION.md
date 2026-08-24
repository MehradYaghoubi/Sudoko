# Samurai Sudoku - Comprehensive Technical Documentation

## Executive Overview
**Samurai Sudoku** is a high-performance, native Windows desktop application built completely in C++17 using the Win32 API and double-buffered Windows Graphics Device Interface (GDI). It provides a lightweight, offline experience for 5-grid overlapping Samurai Sudoku puzzles without external dependencies, browser engines, or heavy framework runtimes.

---

## 1. System Architecture & Component Layout

The codebase is organized into modular components adhering to strict separation of concerns:

```text
├── src/
│   ├── sudoku_engine.h / .cpp  # Core Samurai Sudoku state, solver, generator & JSON I/O
│   ├── ui_renderer.h / .cpp    # Win32 GDI double-buffered rendering pipeline & themes
│   └── main.cpp                # Win32 Application entry point, message loop & event handling
├── tests/
│   └── test_engine.cpp         # Automated C++ unit testing suite
├── Makefile                    # MinGW cross-compilation & test execution build script
├── README.md                   # Quick start & feature guide
└── save.json                   # Local state persistence (generated at runtime)
```

---

## 2. Core Sudoku Engine & Solver Algorithm

### 2.1 Grid Representation & Overlap Mapping
A Samurai Sudoku board consists of **5 overlapping 9x9 Sudoku grids** arranged in a 21x21 coordinate space (369 playable cells out of 441 matrix cells):

- **Grid 0 (Top-Left)**: Offset `(r:0, c:0)`
- **Grid 1 (Top-Right)**: Offset `(r:0, c:12)`
- **Grid 2 (Center)**: Offset `(r:6, c:6)`
- **Grid 3 (Bottom-Left)**: Offset `(r:12, c:0)`
- **Grid 4 (Bottom-Right)**: Offset `(r:12, c:12)`

Overlapping 3x3 regions exist between the Center grid and each of the four corner grids:
- Top-Left overlap: `[6..8, 6..8]`
- Top-Right overlap: `[6..8, 12..14]`
- Bottom-Left overlap: `[12..14, 6..8]`
- Bottom-Right overlap: `[12..14, 12..14]`

Constraint checks (`is_valid_move`) enforce standard Sudoku rules across all sub-grids a cell belongs to simultaneously (row, column, and 3x3 box).

### 2.2 Procedural Generator & Solver
1. **MRV Heuristic Solver**: Uses Depth-First Backtracking with Minimum Remaining Values (MRV) heuristic to prioritize cells with the fewest candidate numbers.
2. **Unique Solution Verification**: Ensures generated puzzles possess exactly one valid solution by searching up to `max_solutions = 2`.
3. **Difficulty Target Levels**:
   - **Easy**: ~220 givens
   - **Medium**: ~180 givens
   - **Hard**: ~150 givens
   - **Expert**: ~130 givens
   - **Master (Monster)**: ~115 givens (maximum valid clue reduction while strictly preserving unique solvability)

### 2.3 Anti-Pattern Randomization
To prevent players from discovering predictable puzzle generation patterns, the engine applies two non-linear post-transformations:
- **Digit Relabeling**: Applies a random 1-to-9 permutation vector `P`.
- **Symmetric Rotations**: Randomly applies 0°, 90°, 180°, or 270° board rotations.

---

## 3. Rendering Pipeline & UI Architecture

### 3.1 Double-Buffered GDI Graphics
To guarantee flicker-free performance across high-resolution screens:
1. Creates an off-screen memory Device Context (`CreateCompatibleDC`) and bitmap (`CreateCompatibleBitmap`).
2. Renders all backgrounds, grid lines, cells, highlights, notes, pop-ups, and HUD controls onto the off-screen DC.
3. Performs a single `BitBlt` copy to the primary window DC during `WM_PAINT`.

### 3.2 Dual UI Themes
- **Modern Theme**: Clean, sleek aesthetic with subtle blue selection highlights and soft backgrounds.
- **Senior / High-Contrast Theme**: Designed for maximum readability and accessibility:
  - High-contrast color palette (deep navy headers, vivid yellow/blue selection highlights, sharp black text).
  - Enlarged bold fonts (+25% scale for cell digits).
  - Prominent clickable/touch HUD action buttons (`Undo`, `Erase`, `Notes`, `Reset`, `Theme`, `Menu`).

---

## 4. Input & Controls

| Input Type | Action | Description |
| :--- | :--- | :--- |
| **Mouse** | Left Click | Select cell / Click HUD buttons / Main Menu interaction |
| **Mouse** | Double/Right Click | Open pop-up number & note selector keypad |
| **Keyboard** | Arrow Keys | Move selection across overlapping sub-grids |
| **Keyboard** | `1` - `9` | Place digit into selected cell |
| **Keyboard** | `Shift + 1..9` / `N` | Place/toggle pencil notes |
| **Keyboard** | `Delete` / `Backspace` | Erase current cell |
| **Keyboard** | `Ctrl + Z` | Multi-level move undo |
| **Keyboard** | `ESC` | Open Game Menu / Return to previous screen |

---

## 5. Persistence & State Serialization

Game state is auto-saved locally after every move and restored on launch via `save.json`:

```json
{
  "difficulty": 1,
  "elapsed_seconds": 124,
  "error_count": 0,
  "senior_mode": true,
  "cells": [
    {"r":0,"c":0,"v":5,"s":5,"g":1,"n":0},
    ...
  ]
}
```

---

## 6. Build & Verification Guide

### 6.1 Compiling with MinGW
```bash
# Build standalone Windows executable (samurai_sudoku.exe)
make

# Run automated unit test suite
make test

# Clean build artifacts
make clean
```

### 6.2 Unit Test Coverage
`tests/test_engine.cpp` verifies:
- Grid overlap mapping correctness.
- 5-difficulty procedural puzzle generation.
- Master difficulty clue reduction targets.
- Game state reset and anti-pattern transformations.
- JSON serialization and deserialization integrity.
