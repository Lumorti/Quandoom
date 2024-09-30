CXX=g++
FLAGS=-Wall -O3 -march=native -pipe -std=c++11
SDL := $(shell sdl2-config --cflags --libs)
LIBS=$(SDL)

# Default target (for non-macOS systems)
all: simulator

simulator: main.cpp
	$(CXX) $(FLAGS) -o simulator main.cpp $(LIBS) -lSDL2 -fopenmp

# macOS-specific target
macos: main.cpp
	clang++ $(FLAGS) -o simulator main.cpp $(LIBS) -I/opt/homebrew/opt/libomp/include -L/opt/homebrew/opt/libomp/lib -lomp -Xpreprocessor -fopenmp

clean:
	rm -f simulator
