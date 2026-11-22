CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -O2 -std=c++17 -Wall
LDFLAGS = -mwindows -static -luser32 -lgdi32 -lkernel32

SRCS = src/main.cpp src/sudoku_engine.cpp src/ui_renderer.cpp
TEST_SRCS = src/sudoku_engine.cpp tests/test_engine.cpp

TARGET = samurai_sudoku.exe
TEST_TARGET = run_tests

all: $(TARGET)

$(TARGET): $(SRCS) src/sudoku_engine.h src/ui_renderer.h
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS) src/sudoku_engine.h
	g++ -O2 -std=c++17 $(TEST_SRCS) -o $(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) *.exe

.PHONY: all clean test

# Native Win32 checks; run on Windows with the MinGW toolchain.
test-ui:
	mkdir -p .qa-preview
	$(CXX) $(CXXFLAGS) src/sudoku_engine.cpp src/ui_renderer.cpp tests/test_layout.cpp -o test_layout.exe -static -lgdi32 -luser32
	./test_layout.exe
	$(CXX) $(CXXFLAGS) src/sudoku_engine.cpp src/ui_renderer.cpp tests/test_input.cpp -o test_input.exe -static -lgdi32 -luser32
	./test_input.exe

.PHONY: test-ui
