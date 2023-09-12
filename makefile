.PHONY: all app temp clean subdirs

SUBDIRS = ODE_solvers
ARGS = -O2
GCC = g++

CFLAGS = -O2 -Wall -mconsole -lm `sdl2-config --libs --cflags`

all: subdirs renderer.o main.o app
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

main.o: main.cpp renderer.h implicitEuler.h ./ODE_solvers/ODESolver.h
	$(GCC) $(ARGS) -c main.cpp -o main.o

app: main.o
	$(GCC) -o $@ $^ $(CFLAGS)