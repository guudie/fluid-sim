#include <iostream>
#include "renderer.h"

renderer::~renderer() {
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool renderer::setup(int w, int h) {
    if(SDL_Init(SDL_INIT_EVERYTHING) != 0) {
        std::cerr << "Error initializing SDL" << std::endl;
        return false;
    }

    if(w > 0 && h > 0) {
        windowWidth = w;
        windowHeight = h;
    } else {
        std::cerr << "Invalid window dimensions" << std::endl;
        return false;
    }

    window = SDL_CreateWindow("cloth sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);
    if(!window) {
        std::cerr << "Error initializing window" << std::endl;
        return false;
    }

    ren = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(!ren) {
        std::cerr << "Error initializing renderer" << std::endl;
        return false;
    }

    return true;
}

void renderer::clearScreen(Uint32 color) const {
    SDL_SetRenderDrawColor(ren, color >> 16, color >> 8, color, 255);
    SDL_RenderClear(ren);
} 

void renderer::render() const {
    SDL_RenderPresent(ren);
}

void renderer::drawLine(glm::vec2 p0, glm::vec2 p1, Uint32 color) const {
    SDL_SetRenderDrawColor(ren, color >> 16, color >> 8, color, 255);
    SDL_RenderDrawLine(ren, p0.x, p0.y, p1.x, p1.y);
}

void renderer::drawPoint(glm::vec2 p, Uint32 color) const {
    SDL_SetRenderDrawColor(ren, color >> 16, color >> 8, color, 255);
    SDL_RenderDrawPoint(ren, p.x, p.y);
}