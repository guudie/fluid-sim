#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <unordered_set>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>
#include <SDL2/SDL.h>
#include "ODE_solvers/implicitEuler.h"
#include "renderer.h"
#include "utils.h"

#define PI 3.14159265359

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

void generateParticles(std::vector<point*>& points, std::unordered_set<point*> grid[16][16], glm::ivec2 from, glm::ivec2 to, float dist, int cellSize) {
    for(int r = from.y; r < to.y; r += dist) {
        for(int c = from.x; c < to.x; c += dist) {
            point* p = new point {
                { c, r },
                { rand() % 11, 0 },
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

int main() {
    const int width = 511, height = 511;
    renderer* _renderer = new renderer();
    bool running = _renderer->setup(width, height);

    const int cellSize = 32;
    const int gridDimX = (width + 1) / cellSize;
    const int gridDimY = (height + 1) / cellSize;
    std::unordered_set<point*> grid[gridDimY][gridDimX];
    std::vector<point*> points;

    glm::ivec2 tl(50, 50);
    glm::ivec2 br(110, 110);
    float radius = 4.0f;
    float dist = 6;

    generateParticles(points, grid, tl, br, dist, cellSize);

    glm::vec2 G(0, 0.5f);

    implicitEuler _integrator([=](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash + G;
    });

    const float K = 250;
    const float h = 8;
    const float h2 = h * h;
    const float h3 = h2 * h;
    const float h6 = pow(h, 6);
    const float p0 = 1;
    const float e = 0.01f;
    const float poly6_constant = 315.0f / (64.0f * PI * pow(h, 6));
    const float spiky_constant = -45 / (PI * h6);

    const float mass = 1;

    Uint32 lastUpd = SDL_GetTicks();
    while(running) {
        Uint32 curTime = SDL_GetTicks();
        if(curTime - lastUpd >= 16) {
            handleInput(running);

            _renderer->clearScreen(0xFF000816);

            // density and pressure
            for(int r = 0; r < gridDimY; r++) {
                for(int c = 0; c < gridDimX; c++) {
                    if(grid[r][c].size() == 0)
                        continue;
                    
                    for(auto& p : grid[r][c]) {
                        // density
                        p->density = 0;
                        for(int ir = std::max(0, r - 1); ir <= std::min(gridDimY - 1, r + 1); ir++) {
                            for(int ic = std::max(0, c - 1); ic <= std::min(gridDimX - 1, c + 1); ic++) {
                                if(grid[ir][ic].size() == 0)
                                    continue;
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
                    if(grid[r][c].size() == 0)
                        continue;
                    
                    for(auto& p : grid[r][c]) {
                        p->acc = { 0, 0 };
                        for(int ir = std::max(0, r - 1); ir <= std::min(gridDimY - 1, r + 1); ir++) {
                            for(int ic = std::max(0, c - 1); ic <= std::min(gridDimX - 1, c + 1); ic++) {
                                if(grid[ir][ic].size() == 0)
                                    continue;
                                for(auto& q : grid[ir][ic]) {
                                    if(q == p)
                                        continue;
                                    const glm::vec2 diff = p->pos - q->pos;
                                    const float r2 = glm::dot(diff, diff);
                                    const float r = sqrt(r2);
                                    const float r3 = r * r2;

                                    if(r > 1e-3 && r < h) {
                                        const float W_spiky = spiky_constant * (h - r) * (h - r);
                                        const float W_lap = -(r3 / (2 * h3)) + (r2 / h2) + (h / (2 * r)) - 1;
                                        p->acc -= (mass / mass) * ((p->pressure + q->pressure) / (2.0f * p->density * q->density)) * W_spiky * (diff / r);
                                        p->acc += e * (mass / mass) * (1.0f / q->density) * (q->vel - p->vel) * W_lap * (diff / r);
                                    }
                                }
                            }
                        }
                        if(isnan(p->acc.x) || isnan(p->acc.y)) {
                            std::cout << "nan encountered in acc";
                            return 0;
                        }
                        p->acc /= p->density;
                    }
                }
            }

            for(auto& p : points) {
                _integrator.integrate(p->pos, p->vel, p->acc, 1);
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
                _renderer->drawCircle(p->pos, radius, 0xFF55AADD);
            }

            _renderer->render();

            lastUpd = curTime;
        }
    }

    std::cout << "Quit program" << std::endl;

    delete _renderer;

    return 0;
}