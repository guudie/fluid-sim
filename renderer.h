#pragma once
#include <SDL2/SDL.h>
#include "glm/glm.hpp"

class renderer {
private:
    int windowWidth, windowHeight;
    SDL_Window* window;
    SDL_Renderer* ren;

public:
    renderer() = default;
    ~renderer();

    int getWidth() const { return windowWidth; }
    int getHeight() const { return windowHeight; }

    bool setup(int w = 0, int h = 0);

    void clearScreen(Uint32 color) const;
    void render() const;

    void drawLine(glm::vec2 p0, glm::vec2 p1, Uint32 color) const;
    void drawPoint(glm::vec2 p, Uint32 color) const;
    void drawCircle(glm::vec2 center, float radius, Uint32 color) const;
};