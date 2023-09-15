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

inline static void capVelocity(glm::vec2& vel, float maxVel) {
    float len = glm::length(vel);
    if(len < 1e-6) {
        vel = { 0, 0 };
        return;
    }
    if(len > maxVel)
        vel *= maxVel / len;
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
    glm::ivec2 br(200, 200);
    float radius = 4.0f;
    float dist = 8;

    generateParticles(points, grid, tl, br, dist, cellSize);

    glm::vec2 G(0, 0.5f);

    implicitEuler _integrator([=](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash + G;
    });

    const float K = 3;
    const float h = 8;
    const float h2 = h * h;
    const float h3 = h2 * h;
    const float h6 = pow(h, 6);
    const float p0 = 1;
    const float e = 0.1f;
    const float poly6_constant = 315.0f / (64.0f * PI * pow(h, 6));
    const float spiky_constant = -45 / (PI * h6);
    const float viscosity_lap_constant = 45 / (PI * h6);

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
                        // for(int ir = std::max(0, r - 1); ir <= std::min(gridDimY - 1, r + 1); ir++) {
                        //     for(int ic = std::max(0, c - 1); ic <= std::min(gridDimX - 1, c + 1); ic++) {
                        //         for(auto& q : grid[ir][ic]) {
                        //             if(q == p)
                        //                 continue;
                        //             const glm::vec2 diff = p->pos - q->pos;
                        //             const float r2 = glm::dot(diff, diff);
                        //             const float r = sqrt(r2);
                        //             const float r3 = r * r2;

                        //             if(r > 1e-3 && r < h) {
                        //                 const float W_spiky = spiky_constant * (h - r) * (h - r);
                        //                 // const float W_lap = -(r3 / (2 * h3)) + (r2 / h2) + (h / (2 * r)) - 1;
                        //                 const float W_lap = viscosity_lap_constant * (h - r);
                        //                 p->acc -= (mass / mass) * ((p->pressure + q->pressure) / (2.0f * p->density * q->density)) * W_spiky * (diff / r);
                        //                 // p->acc += e * (mass / mass) * (1.0f / q->density) * (q->vel - p->vel) * W_lap;
                        //             }
                        //         }
                        //     }
                        // }
                        if(isnan(p->acc.x) || isnan(p->acc.y)) {
                            std::cout << "nan encountered in acc";
                            return 0;
                        }
                        // p->acc /= p->density;
                    }
                }
            }

            // integrate movements
            for(auto& p : points) {
                // _integrator.integrate(p->pos, p->vel, p->acc, 1);

                // calculate velocity
                _integrator.integrateStep1(p->pos, p->vel, p->acc, 1);
                capVelocity(p->vel, 20.0f);

                // calculate and constrain position
//                 glm::vec2 prevPos = p->pos;
//                 _integrator.integrateStep2(p->pos, p->vel, 1);
//                 for(int r = std::max(0, p->gridIdx.y - 1); r <= std::min(gridDimY - 1, p->gridIdx.y + 1); r++) {
//                     for(int c = std::max(0, p->gridIdx.x - 1); c <= std::min(gridDimX - 1, p->gridIdx.x + 1); c++) {
//                         for(auto& q : grid[r][c]) {
//                             if(q == p)
//                                 continue;
//                             if(glm::length(q->pos - p->pos) < 8) {
//                                 p->pos = prevPos;
//                                 goto skip_loop;
//                             }
//                         }
//                     }
//                 }
// skip_loop:
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

            // constrain distance
            // for(int i = 0; i < 3; i++) {
            //     for(int r = 0; r < gridDimY; r++) {
            //         for(int c = 0; c < gridDimX; c++) {
            //             if(grid[r][c].size() == 0)
            //                 continue;
                        
            //             for(auto& p : grid[r][c]) {
            //                 for(int ir = std::max(0, r - 1); ir <= std::min(gridDimY - 1, r + 1); ir++) {
            //                     for(int ic = std::max(0, c - 1); ic <= std::min(gridDimX - 1, c + 1); ic++) {
            //                         if(grid[ir][ic].size() == 0)
            //                             continue;
            //                         std::vector<point*> tmp_points;
            //                         for(auto& q : grid[ir][ic]) {
            //                             if(glm::length(q->pos - p->pos) < 8)
            //                                 tmp_points.push_back(q);
            //                         }
            //                         for(auto& q : tmp_points) {
            //                             if(q == p)
            //                                 continue;
            //                             glm::vec2 seg = q->pos - p->pos;
            //                             float len = glm::length(seg);
            //                             if(len < 1e-6) {
            //                                 q->pos.y--;
            //                                 seg = q->pos - p->pos;
            //                                 len = glm::length(seg);
            //                             }
            //                             if(seg.y > 0) {
            //                                 continue;
            //                             }
            //                             // q->vel += (8 / len - 1) * seg;
            //                             q->pos += (8 / len - 1) * seg;
            //                             resolveOutOfBounds(*q, width, height);
            //                             glm::ivec2 newIdx = { q->pos.x / cellSize, q->pos.y / cellSize };
            //                             if(q->gridIdx != newIdx) {
            //                                 grid[q->gridIdx.y][q->gridIdx.x].erase(q);
            //                                 grid[newIdx.y][newIdx.x].insert(q);
            //                                 q->gridIdx = newIdx;
            //                             }
            //                         }
            //                     }
            //                 }
            //             }
            //         }
            //     }
            // }

            int cnt = 0;
            for(auto& p : points)
                _renderer->drawCircle(p->pos, radius, cnt++ == points.size() - 1 ? 0xFFFF5555 : 0xFF55AADD);

            _renderer->render();

            lastUpd = curTime;
        }
    }

    std::cout << "Quit program" << std::endl;

    delete _renderer;

    return 0;
}