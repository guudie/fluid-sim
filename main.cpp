#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <SDL2/SDL.h>
#include "ODE_solvers/implicitEuler.h"
#include "renderer.h"
#include "mouse.h"
#include "utils.h"

#define PI 3.14159265359

inline static void handleInput(bool& running, mouse* _mouse) {
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
        case SDL_MOUSEMOTION:
            x = event.motion.x;
            y = event.motion.y;
            _mouse->updatePos(x, y);
            break;
        case SDL_MOUSEBUTTONDOWN:
            SDL_GetMouseState(&x, &y);
            _mouse->updatePos(x, y);
            if (!_mouse->getLB() && event.button.button == SDL_BUTTON_LEFT) 
            {
                _mouse->setLB(true);
            }
            if (!_mouse->getRB() && event.button.button == SDL_BUTTON_RIGHT) 
            {
                _mouse->setRB(true);
            }
            if (!_mouse->getSBX2() && event.button.button == SDL_BUTTON_X2) 
            {
                _mouse->setSBX2(true);
            }
            break;
        case SDL_MOUSEBUTTONUP: 
            if (_mouse->getLB() && event.button.button == SDL_BUTTON_LEFT)
            {
                _mouse->setLB(false);
            }
            if (_mouse->getRB() && event.button.button == SDL_BUTTON_RIGHT)
            {
                _mouse->setRB(false);
            }
            if (_mouse->getSBX2() && event.button.button == SDL_BUTTON_X2) 
            {
                _mouse->setSBX2(false);
            }
            break;
        }
    }
}

void generateParticles(std::vector<point*>& points, std::unordered_set<point*> grid[16][16], glm::ivec2 from, glm::ivec2 to, float dist, int cellSize) {
    for(int r = from.y; r < to.y; r += dist) {
        for(int c = from.x; c < to.x; c += dist) {
            point* p = new point {
                { c, r },
                { rand() % 1, 0 },
                { 0, 0 },
                { c / cellSize, r / cellSize },
                0.0f,
                0.0f,
                false
            };
            points.push_back(p);
            grid[p->gridIdx.y][p->gridIdx.x].insert(p);
        }
    }
}

inline static void capMagnitude(glm::vec2& v, float maxMag) {
    float len = glm::length(v);
    if(len < 1e-6) {
        v = { 0, 0 };
        return;
    }
    if(len > maxMag)
        v *= maxMag / len;
}

int main() {
    const int width = 511, height = 511;
    renderer* _renderer = new renderer();
    bool running = _renderer->setup(width, height);
    float dt = 1.0f;

    mouse* _mouse = new mouse();

    const int cellSize = 32;
    const int gridDimX = (width + 1) / cellSize;
    const int gridDimY = (height + 1) / cellSize;
    std::unordered_set<point*> grid[gridDimY][gridDimX];
    std::vector<point*> points;

    glm::ivec2 tl(50, 50);
    glm::ivec2 br(125, 200);
    float radius = 4.0f;
    float dist = 7;

    generateParticles(points, grid, tl, br, dist, cellSize);

    tl = { 300, 50 };
    br = { 375, 200 };
    generateParticles(points, grid, tl, br, dist, cellSize);

    tl = { 250, 50 };
    br = { 325, 200 };
    bool once = false;

    glm::vec2 G(0, 0.01f);

    implicitEuler _integrator([=](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash + G;
    });

    const float K = 250;
    const float h = 8;
    const float h2 = h * h;
    const float h6 = pow(h, 0);
    const float p0 = 1;
    const float e = 0.001f;
    const float poly6_constant = 315.0f / (64.0f * PI * pow(h, 0));
    const float spiky_constant = -45 / (PI * h6);
    const float viscosity_lap_constant = 45 / (PI * h6);

    const float mass = 1;

    Uint32 lastUpd = SDL_GetTicks();
    while(running) {
        Uint32 curTime = SDL_GetTicks();
        if(curTime - lastUpd >= 16) {
            handleInput(running, _mouse);
            if(_mouse->getRB() && !once) {
                generateParticles(points, grid, tl, br, dist, cellSize);
                once = true;
            }

            _renderer->clearScreen(0xFF000816);

            for(int i = 0; i < 1; i++) {
                // density and pressure
                for(int r = 0; r < gridDimY; r++) {
                    for(int c = 0; c < gridDimX; c++) {
                        for(auto& p : grid[r][c]) {
                            // density
                            p->density = 0;
                            for(int ir = std::max(0, r - 1); ir <= std::min(gridDimY - 1, r + 1); ir++) {
                                for(int ic = std::max(0, c - 1); ic <= std::min(gridDimX - 1, c + 1); ic++) {
                                    for(auto& q : grid[ir][ic]) {
                                        const glm::vec2 diff = p->pos - q->pos;
                                        const float r2 = glm::dot(diff, diff);
                                        if(r2 < h2) {
                                            const float W = poly6_constant * pow(h2 - r2, 3);
                                            p->density += mass * W;
                                        }
                                    }
                                }
                            }
                            if(isnan(p->density)) {
                                std::cout << "nan encountered in density";
                                return 0;
                            }
                            p->density = std::max(p0, p->density);

                            // pressure
                            p->pressure = K * (p->density - p0);
                            if(isnan(p->pressure)) {
                                std::cout << "nan encountered in pressure";
                                return 0;
                            }
                        }
                    }
                }

                // acceleration
                for(int r = 0; r < gridDimY; r++) {
                    for(int c = 0; c < gridDimX; c++) {
                        for(auto& p : grid[r][c]) {
                            p->acc = { 0, 0 };
                            for(int ir = std::max(0, r - 1); ir <= std::min(gridDimY - 1, r + 1); ir++) {
                                for(int ic = std::max(0, c - 1); ic <= std::min(gridDimX - 1, c + 1); ic++) {
                                    for(auto& q : grid[ir][ic]) {
                                        if(q == p)
                                            continue;
                                        const glm::vec2 diff = p->pos - q->pos;
                                        const float r2 = glm::dot(diff, diff);
                                        const float r = sqrt(r2);

                                        if(r > 1e-3 && r < h) {
                                            const float W_spiky = spiky_constant * (h - r) * (h - r);
                                            const float W_lap = viscosity_lap_constant * (h - r);
                                            p->acc -= (mass / mass) * ((p->pressure + q->pressure) / (2.0f * p->density * q->density)) * W_spiky * (diff / r);
                                            p->acc += e * (mass / mass) * (1.0f / q->density) * (q->vel - p->vel) * W_lap;
                                        }
                                    }
                                }
                            }
                            // if(p->pos.y >= height-10) {
                            //     const glm::vec2 diff = { 0, 2 * radius - p->pos.y + height - 10 };
                            //     const float r2 = glm::dot(diff, diff);
                            //     const float r = sqrt(r2);
                            //     if(r > 1e-3 && r < h) {
                            //         const float W_spiky = spiky_constant * (h - r) * (h - r);
                            //         p->acc -= p->pressure / (2.0f * p->density * 1.0f) * W_spiky * (diff / r);
                            //     }
                            // }
                            if(isnan(p->acc.x) || isnan(p->acc.y)) {
                                std::cout << "nan encountered in acc";
                                return 0;
                            }
                            // capMagnitude(p->acc, 0.5f);
                        }
                    }
                }

                // integrate movements
                for(auto& p : points) {
                    // _integrator.integrate(p->pos, p->vel, p->acc, dt);

                    // calculate velocity
                    if(_mouse->getLB()) {
                        glm::vec2 toMouse = _mouse->getPos() - p->pos;
                        if(glm::dot(toMouse, toMouse) < 32 * 32)
                            p->vel += 0.01f * _mouse->getDiff();
                    }
                    _integrator.integrateStep1(p->pos, p->vel, p->acc, dt);
                    capMagnitude(p->vel, 0.7f);
                    
                    _integrator.integrateStep2(p->pos, p->vel, dt);
                    resolveOutOfBounds(*p, width, height);

                    if(isnan(p->pos.x) || isnan(p->pos.y)) {
                        std::cout << "nan encountered in position";
                        return 0;
                    }
                    glm::ivec2 newIdx = { p->pos.x / cellSize, p->pos.y / cellSize };
                    if(p->gridIdx != newIdx) {
                        if(newIdx.x < 0 || newIdx.x >= gridDimX || newIdx.y < 0 || newIdx.y >= gridDimY) {
                            std::cout << "seriously???";
                            return 0;
                        }
                        grid[p->gridIdx.y][p->gridIdx.x].erase(p);
                        grid[newIdx.y][newIdx.x].insert(p);
                        p->gridIdx = newIdx;
                    }
                }
            }

            for(auto& p : points)
                _renderer->drawCircle(p->pos, radius, 0xFF55AADD);

            _renderer->render();

            lastUpd = curTime;
        }
    }

    for(auto& p : points)
        delete p;
    delete _renderer;
    delete _mouse;

    std::cout << "Quit program" << std::endl;

    return 0;
}