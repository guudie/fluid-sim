.PHONY: all app temp clean subdirs

EXT =
WINOPT = 
ifeq ($(OS), Windows_NT)
	EXT = .exe
	WINOPT = -mconsole
endif
SUBDIRS = ODE_solvers
ARGS = -O2
GCC = g++

OMP = -fopenmp
CFLAGS = $(WINOPT) $(OMP) -O2 -Wall -lm `sdl2-config --libs` -lconfig++

all: subdirs renderer.o mouse.o utils.o main.o app$(EXT)
clean:
	-rm *.o *.exe; \
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

renderer.o: renderer.h renderer.cpp
	$(GCC) $(ARGS) -c renderer.cpp -o renderer.o

mouse.o: mouse.h mouse.cpp
	$(GCC) $(ARGS) -c mouse.cpp -o mouse.o

utils.o: utils.h utils.cpp
	$(GCC) $(ARGS) -c utils.cpp -o utils.o

main.o: main.cpp renderer.h mouse.h utils.h ./ODE_solvers/implicitEuler.h ./ODE_solvers/ODESolver.h
	$(GCC) $(ARGS) $(OMP) -c main.cpp -o main.o

app$(EXT): main.o renderer.o mouse.o utils.o ./ODE_solvers/ode_joined.o
	$(GCC) -o $@ $^ $(CFLAGS)