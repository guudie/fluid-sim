.PHONY: all app clean pack pack_r

# --- toggles ----------------------------------------------------------------
CONSOLE_OUTPUT = true
DEBUG          = false
STATIC_LINK    = false

# --- build mode -------------------------------------------------------------
BUILD      = release
OPT        = -O2
DEBUGFLAGS = -g0
ifeq ($(DEBUG), true)
	BUILD      = debug
	OPT        = -O0
	DEBUGFLAGS = -ggdb
endif

# --- platform ---------------------------------------------------------------
EXT    =
WINOPT =
ifeq ($(OS), Windows_NT)
	EXT = .exe
	ifeq ($(CONSOLE_OUTPUT), true)
		WINOPT = -mconsole
	endif
endif

# --- layout -----------------------------------------------------------------
ODEDIR       = ODE_solvers
BASE_OBJDIR  = obj
BASE_BIN     = bin
OBJDIR       = $(BASE_OBJDIR)/$(BUILD)
BIN          = $(BASE_BIN)/$(BUILD)

# --- toolchain / flags ------------------------------------------------------
GCC = g++
OMP = -fopenmp

LIBS    = `sdl2-config --libs` -lconfig++
LCFGFLAG =
ifeq ($(STATIC_LINK), true)
	LIBS     = `sdl2-config --static-libs` -lconfig++ --static
	LCFGFLAG = -DLIBCONFIGXX_STATIC
endif

# Compile flags shared by every translation unit.
# -MMD -MP auto-generates .d files so header edits trigger rebuilds.
CXXFLAGS = $(OPT) -Wall $(DEBUGFLAGS) $(OMP) $(LCFGFLAG) -MMD -MP
# Link-only flags / libraries.
LDFLAGS  = $(WINOPT) $(DEBUGFLAGS) $(OMP)
LDLIBS   = $(LIBS) -lm

# --- sources ----------------------------------------------------------------
SRCS = \
	main.cpp \
	renderer.cpp \
	mouse.cpp \
	utils.cpp \
	fluid_sim.cpp \
	$(ODEDIR)/ODESolver.cpp \
	$(ODEDIR)/velocityVerlet.cpp \
	$(ODEDIR)/verlet.cpp \
	$(ODEDIR)/implicitEuler.cpp

OBJS = $(SRCS:%.cpp=$(OBJDIR)/%.o)
DEPS = $(OBJS:.o=.d)

# --- targets ----------------------------------------------------------------
all: app
app: $(BIN)/app$(EXT)

clean:
	-rm -r $(BASE_OBJDIR)/* $(BASE_BIN)/*

# Compile: one pattern rule for every .cpp (top-level and ODE_solvers/).
# $(@D) is the object's dir, created on demand.
$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(@D)
	$(GCC) $(CXXFLAGS) -c $< -o $@

# Link.
$(BIN)/app$(EXT): $(OBJS)
	@mkdir -p $(@D)
	$(GCC) $(LDFLAGS) -o $@ $^ $(LDLIBS)

-include $(DEPS)

# --- misc -------------------------------------------------------------------
pack:
	$(MAKE) pack_r DEBUG=false

pack_r: all
	@7z u application.zip ./$(BIN)/app$(EXT) config
