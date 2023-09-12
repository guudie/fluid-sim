#define SDL_MAIN_HANDLED
#include <iostream>
#include <glm/glm.hpp>
#include <SDL2/SDL.h>
#include "ODE_solvers/implicitEuler.h"
#include "renderer.h"

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
    int width = 512, height = 512;
    renderer* _renderer = new renderer();
    bool running = _renderer->setup(width, height);

    glm::vec2 p(50, 50);
    float radius = 6.0f;

    Uint32 lastUpd = SDL_GetTicks();
    while(running) {
        Uint32 curTime = SDL_GetTicks();
        if(curTime - lastUpd >= 16) {
            handleInput(running);

            _renderer->clearScreen(0xFF000816);

            _renderer->drawCircle(p, radius, 0xFF55AADD);
            // _renderer->drawPoint(p, 0xFFFFFFFF);

            _renderer->render();

            lastUpd = curTime;
        }
    }

    std::cout << "Quit program" << std::endl;

    delete _renderer;

    return 0;
}