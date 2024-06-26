.PHONY: all app temp clean subdirs

EXT =
WINOPT =

CONSOLE_OUTPUT = true

DEBUG = false
DEBUGFLAGS = -g0
ifeq ($(DEBUG), true)
	DEBUGFLAGS = -ggdb
endif

ifeq ($(OS), Windows_NT)
	EXT = .exe
	ifeq ($(CONSOLE_OUTPUT), true)
		WINOPT = -mconsole
	endif
endif
SUBDIRS = ODE_solvers
ARGS = -O2
GCC = g++

OMP = -fopenmp
LIBS = `sdl2-config --libs` -lconfig++
LCFGFLAG =
STATIC_LINK = false
ifeq ($(STATIC_LINK), true)
	LIBS = `sdl2-config --static-libs` -lconfig++ --static
	LCFGFLAG = -DLIBCONFIGXX_STATIC
endif
CFLAGS = $(WINOPT) $(OMP) -O2 -Wall -lm $(LIBS)

all: subdirs renderer.o mouse.o utils.o fluid_sim.o main.o app$(EXT)
clean:
	-rm *.o *.exe; \
	for dir in $(SUBDIRS); do \
		$(MAKE) DEBUG=$(DEBUG) -C $$dir clean; \
	done

subdirs:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir; \
	done

renderer.o: renderer.h renderer.cpp
	$(GCC) $(ARGS) $(DEBUGFLAGS) -c renderer.cpp -o renderer.o

mouse.o: mouse.h mouse.cpp
	$(GCC) $(ARGS) $(DEBUGFLAGS) -c mouse.cpp -o mouse.o

utils.o: utils.h utils.cpp global.h
	$(GCC) $(ARGS) $(DEBUGFLAGS) $(LCFGFLAG) -c utils.cpp -o utils.o

fluid_sim.o: fluid_sim.h fluid_sim.cpp renderer.h mouse.h utils.h ./ODE_solvers/ODESolver.h
	$(GCC) $(ARGS) $(DEBUGFLAGS) $(LCFGFLAG) $(OMP) -c fluid_sim.cpp -o fluid_sim.o

main.o: main.cpp renderer.h mouse.h utils.h fluid_sim.h ./ODE_solvers/implicitEuler.h global.h
	$(GCC) $(ARGS) $(DEBUGFLAGS) $(LCFGFLAG) $(OMP) -c main.cpp -o main.o

app$(EXT): main.o renderer.o mouse.o utils.o fluid_sim.o ./ODE_solvers/ode_joined.o
	$(GCC) $(DEBUGFLAGS) -o $@ $^ $(CFLAGS)