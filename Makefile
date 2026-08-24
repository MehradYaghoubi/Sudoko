CXX = x86_64-w64-mingw32-g++
CXXFLAGS = -O2 -std=c++17 -Wall
LDFLAGS = -mwindows -static -luser32 -lgdi32 -lkernel32

SRCS = src/main.cpp src/sudoku_engine.cpp src/ui_renderer.cpp
TEST_SRCS = src/sudoku_engine.cpp tests/test_engine.cpp

TARGET = samurai_sudoku.exe
TEST_TARGET = run_tests

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

test: $(TEST_TARGET)
	./$(TEST_TARGET)

$(TEST_TARGET): $(TEST_SRCS)
	g++ -O2 -std=c++17 $(TEST_SRCS) -o $(TEST_TARGET)

clean:
	rm -f $(TARGET) $(TEST_TARGET) *.exe

.PHONY: all clean test
