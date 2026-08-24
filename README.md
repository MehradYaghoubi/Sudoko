# Samurai Sudoku (Windows Portable)

A lightweight, high-performance, offline **Samurai Sudoku** desktop application for Windows (Windows 8.1 / 10 / 11).

Built natively in C++ using Win32 API and GDI graphics with zero external dependencies or heavy web frameworks.

---

## Key Features

- **Classic 5-Grid Samurai Sudoku Layout**: Top-Left, Top-Right, Center, Bottom-Left, Bottom-Right with overlapping 3x3 regions.
- **Unique Solution Generator**: Guarantees procedural generation of puzzles with unique solutions.
- **3 Difficulty Levels**: Easy (~220 givens), Medium (~180 givens), Hard (~150 givens).
- **Fullscreen Window Experience**: Opens immediately in seamless borderless fullscreen window mode.
- **Controls**:
  - **Keyboard**: Arrow keys for navigation, `1`-`9` for digit placement, `Delete` / `Backspace` to erase, `Shift + 1-9` or `N` key to toggle pencil notes mode, `Ctrl + Z` to Undo.
  - **Mouse & Pop-up Selector**: Click any empty cell to select it. Click again or right-click to open a pop-up number/note selector.
- **Loading Scene**: Asynchronous puzzle generation with a clean loading overlay.
- **Auto-Save & Resume**: State automatically saved to `save.json` next to the executable on every move and restored on startup.
- **ESC Game Menu**: Press `ESC` anytime to open the simple in-game menu (Resume, New Game, Quit).

---

## Requirements & Building

### Pre-built Binary
The standalone Portable executable is generated as:
```text
samurai_sudoku.exe
```
No installation required. Simply run `samurai_sudoku.exe` on Windows 8.1+.

### Building from Source (Linux Cross-Compilation)
Dependencies: `g++`, `mingw-w64`, `make`.

Run:
```bash
make
```

To run unit tests:
```bash
make test
```

To clean build artifacts:
```bash
make clean
```
