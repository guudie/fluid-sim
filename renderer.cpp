#include <iostream>
#include "renderer.h"

renderer::~renderer() {
    SDL_DestroyRenderer(ren);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

int renderer::getWidth() const {
    return windowWidth;
}

int renderer::getHeight() const {
    return windowHeight;
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

    window = SDL_CreateWindow("water sim", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, 0);
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

void renderer::drawCircle(glm::vec2 center, float radius, Uint32 color) const {
    int centerX = center.x;
    int centerY = center.y;
    const int diameter = radius * 2;
    int x = radius - 1;
    int y = 0;
    int tx = 1;
    int ty = 1;
    int error = tx - diameter;

    SDL_SetRenderDrawColor(ren, color >> 16, color >> 8, color, 255);
    while (x >= y)
    {
        SDL_RenderDrawPoint(ren, centerX + x, centerY - y);
        SDL_RenderDrawPoint(ren, centerX + x, centerY + y);
        SDL_RenderDrawPoint(ren, centerX - x, centerY - y);
        SDL_RenderDrawPoint(ren, centerX - x, centerY + y);
        SDL_RenderDrawPoint(ren, centerX + y, centerY - x);
        SDL_RenderDrawPoint(ren, centerX + y, centerY + x);
        SDL_RenderDrawPoint(ren, centerX - y, centerY - x);
        SDL_RenderDrawPoint(ren, centerX - y, centerY + x);

        if (error <= 0)
        {
            ++y;
            error += ty;
            ty += 2;
        }

        if (error > 0)
        {
            --x;
            tx += 2;
            error += tx - diameter;
        }
    }
}

void renderer::drawFilledCircle(glm::vec2 center, float radius, Uint32 color) const {
    SDL_SetRenderDrawColor(ren, color >> 16, color >> 8, color, 255);
    for (int w = 0; w < radius * 2; w++) {
        for (int h = 0; h < radius * 2; h++) {
            int dx = radius - w; // horizontal offset
            int dy = radius - h; // vertical offset
            if ((dx*dx + dy*dy) <= (radius * radius)) {
                SDL_RenderDrawPoint(ren, center.x + dx, center.y + dy);
            }
        }
    }
}