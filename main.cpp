#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "ODE_solvers/implicitEuler.h"
#include "renderer.h"
#include "utils.h"

inline static void handleInput(bool& running) {
    SDL_Event event;
    int x, y;
    while(SDL_PollEvent(&event)) {
        switch(event.type) {
        case SDL_QUIT:
            running = false;
            break;
        case SDL_KEYDOWN:
            if(event.key.keysym.sym == SDLK_ESCAPE)
                running = false;
            break;
        }
    }
}

int main() {
    const int width = 512, height = 512;
    renderer* _renderer = new renderer();
    bool running = _renderer->setup(width, height);

    const int cellSize = 32;
    const int gridDimX = width / cellSize;
    const int gridDimY = height / cellSize;
    std::unordered_set<point*> grid[gridDimX][gridDimY];

    point p = {
        { 50, 50 },
        { 0, 0 },
        { 0, 0.5f },
        false
    };
    float radius = 4.0f;

    implicitEuler _integrator([](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash;
    });

    Uint32 lastUpd = SDL_GetTicks();
    while(running) {
        Uint32 curTime = SDL_GetTicks();
        if(curTime - lastUpd >= 16) {
            handleInput(running);

            _renderer->clearScreen(0xFF000816);

            _integrator.integrate(p.pos, p.vel, p.acc, 1);
            resolveOutOfBounds(p, width, height);

            _renderer->drawCircle(p.pos, radius, 0xFF55AADD);

            _renderer->render();

            lastUpd = curTime;
        }
    }

    std::cout << "Quit program" << std::endl;

    delete _renderer;

    return 0;
}