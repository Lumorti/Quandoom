CXX=g++
FLAGS=-Wall -O3 -fmax-errors=3 -march=native -pipe
SDL := $(shell sdl2-config --cflags)
LIBS=$(SDL)

all: simulator

simulator: main.cpp
	$(CXX) $(FLAGS) -o simulator main.cpp $(LIBS) -lSDL2 -fopenmp



