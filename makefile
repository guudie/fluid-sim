.PHONY: all app clean

EXT =
WINOPT =

CONSOLE_OUTPUT = true
BUILD = release

DEBUG = false
DEBUGFLAGS = -g0
ifeq ($(DEBUG), true)
	BUILD = debug
	DEBUGFLAGS = -ggdb
endif

ifeq ($(OS), Windows_NT)
	EXT = .exe
	ifeq ($(CONSOLE_OUTPUT), true)
		WINOPT = -mconsole
	endif
endif

ODEDIR = ODE_solvers

BASE_OBJDIR = obj
BASE_BIN = bin
OBJDIR = $(BASE_OBJDIR)/$(BUILD)
BIN = $(BASE_BIN)/$(BUILD)

ODE_OBJDIR = $(OBJDIR)/$(ODEDIR)

ARGS = -O2 -Wall
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

# Common compile flags shared by every translation unit.
CXXFLAGS = $(ARGS) $(DEBUGFLAGS)

OBJS = \
	$(OBJDIR)/main.o \
	$(OBJDIR)/renderer.o \
	$(OBJDIR)/mouse.o \
	$(OBJDIR)/utils.o \
	$(OBJDIR)/fluid_sim.o \
	$(ODE_OBJDIR)/ODESolver.o \
	$(ODE_OBJDIR)/velocityVerlet.o \
	$(ODE_OBJDIR)/verlet.o \
	$(ODE_OBJDIR)/implicitEuler.o

all: app
app: $(BIN)/app$(EXT)

clean:
	-rm -r $(BASE_OBJDIR)/* $(BASE_BIN)/*

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(ODE_OBJDIR):
	mkdir -p $(ODE_OBJDIR)

$(BIN):
	mkdir -p $(BIN)

# --- main sources -----------------------------------------------------------
$(OBJDIR)/renderer.o: renderer.h renderer.cpp | $(OBJDIR)
	$(GCC) $(CXXFLAGS) -c renderer.cpp -o $@

$(OBJDIR)/mouse.o: mouse.h mouse.cpp | $(OBJDIR)
	$(GCC) $(CXXFLAGS) -c mouse.cpp -o $@

$(OBJDIR)/utils.o: utils.h utils.cpp global.h | $(OBJDIR)
	$(GCC) $(CXXFLAGS) $(LCFGFLAG) -c utils.cpp -o $@

$(OBJDIR)/fluid_sim.o: fluid_sim.h fluid_sim.cpp renderer.h mouse.h utils.h $(ODEDIR)/ODESolver.h | $(OBJDIR)
	$(GCC) $(CXXFLAGS) $(LCFGFLAG) $(OMP) -c fluid_sim.cpp -o $@

$(OBJDIR)/main.o: main.cpp renderer.h mouse.h utils.h fluid_sim.h $(ODEDIR)/implicitEuler.h global.h | $(OBJDIR)
	$(GCC) $(CXXFLAGS) $(LCFGFLAG) $(OMP) -c main.cpp -o $@

# --- ODE solvers ------------------------------------------------------------
$(ODE_OBJDIR)/ODESolver.o: $(ODEDIR)/ODESolver.h $(ODEDIR)/ODESolver.cpp | $(ODE_OBJDIR)
	$(GCC) $(CXXFLAGS) -c $(ODEDIR)/ODESolver.cpp -o $@

$(ODE_OBJDIR)/velocityVerlet.o: $(ODEDIR)/ODESolver.h $(ODEDIR)/velocityVerlet.h $(ODEDIR)/velocityVerlet.cpp | $(ODE_OBJDIR)
	$(GCC) $(CXXFLAGS) -c $(ODEDIR)/velocityVerlet.cpp -o $@

$(ODE_OBJDIR)/verlet.o: $(ODEDIR)/ODESolver.h $(ODEDIR)/verlet.h $(ODEDIR)/verlet.cpp | $(ODE_OBJDIR)
	$(GCC) $(CXXFLAGS) -c $(ODEDIR)/verlet.cpp -o $@

$(ODE_OBJDIR)/implicitEuler.o: $(ODEDIR)/ODESolver.h $(ODEDIR)/implicitEuler.h $(ODEDIR)/implicitEuler.cpp | $(ODE_OBJDIR)
	$(GCC) $(CXXFLAGS) -c $(ODEDIR)/implicitEuler.cpp -o $@

# --- link -------------------------------------------------------------------
$(BIN)/app$(EXT): $(OBJS) | $(BIN)
	$(GCC) $(DEBUGFLAGS) -o $@ $^ $(CFLAGS)
