# Samurai Sudoku for Windows

An offline desktop implementation of Samurai Sudoku in C++ using the Win32 API and GDI. The board contains five overlapping 9×9 grids.

## Features

- Procedurally generated puzzles with a unique-solution check
- Easy, medium and hard difficulty settings
- Keyboard and mouse input, pencil notes and undo
- Borderless fullscreen and windowed play
- Automatic save and resume through save.json
- English and Persian interface support

## Run on Windows

Download samurai_sudoku.exe from this repository and run it on Windows 8.1 or later. It is portable and does not require an installer. Windows may show a security prompt for an unsigned executable; run only binaries you trust.

## Build from source

The Makefile uses g++, make and a MinGW-w64 cross-compiler.

```bash
make
make test
```

Use make clean to remove generated build files. Source is under src/ and engine tests are under tests/. This project targets Win32 and is not a native Linux application.

## Controls

| Action | Control |
| --- | --- |
| Move selection | Arrow keys |
| Enter a digit | 1–9 |
| Erase a value | Delete or Backspace |
| Toggle notes | N or Shift + digit |
| Undo | Ctrl + Z |
| Open game menu | Esc |

Select an empty cell with the mouse; click again or right-click to open the number and notes selector.

Detailed implementation notes are in [TECHNICAL_DOCUMENTATION.md](TECHNICAL_DOCUMENTATION.md). Save data is written beside the executable, so that directory must be writable.
