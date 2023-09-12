.PHONY: all temp clean

SUBDIRS = ODE_solvers
ARGS = -O2
GCC = g++

CFLAGS = -O2 -Wall -mconsole -lm `sdl2-config --libs --cflags`

main.o: main.cpp
	$(GCC) $(ARGS) -c main.cpp -o main.o

app: main.o
	$(GCC) -o $@ $^ $(CFLAGS)