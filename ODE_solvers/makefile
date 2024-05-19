.PHONY: all clean

DEBUG = false

ARGS = -O2 -Wall
GCC = g++
LD = ld
LDFLAGS = -r -O2

DEBUGFLAGS = -g0
ifeq ($(DEBUG), true)
	DEBUGFLAGS = -ggdb
endif

all: ODESolver.o velocityVerlet.o verlet.o ode_joined.o
clean:
	-rm *.o

ODESolver.o: ODESolver.h ODESolver.cpp
	$(GCC) $(ARGS) $(DEBUGFLAGS) -c ODESolver.cpp -o ODESolver.o

velocityVerlet.o: ODESolver.h velocityVerlet.h velocityVerlet.cpp
	$(GCC) $(ARGS) $(DEBUGFLAGS) -c velocityVerlet.cpp -o velocityVerlet.o

verlet.o: ODESolver.h verlet.h verlet.cpp
	$(GCC) $(ARGS) $(DEBUGFLAGS) -c verlet.cpp -o verlet.o

implicitEuler.o: ODESolver.h implicitEuler.h implicitEuler.cpp
	$(GCC) $(ARGS) $(DEBUGFLAGS) -c implicitEuler.cpp -o implicitEuler.o

ode_joined.o: ODESolver.o velocityVerlet.o verlet.o implicitEuler.o
	$(LD) $(LDFLAGS) -o $@ $^