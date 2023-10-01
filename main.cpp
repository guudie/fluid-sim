#define SDL_MAIN_HANDLED
#include <iostream>
#include <vector>
#include <unordered_set>
#include <SDL2/SDL.h>
#include <omp.h>
#include <libconfig.h++>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
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

void generateParticles(std::vector<point*>& points, std::unordered_set<point*> grid[][32], glm::ivec2 from, glm::ivec2 to, float dist, int cellSize) {
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
    std::string configPath = "config/general.cfg";
    std::string utilsConfigPath = "config/utils.cfg";

    renderer* _renderer = new renderer();
    bool running = _renderer->setup(width, height);

    mouse* _mouse = new mouse();

    libconfig::Config cfg;
    try {
        cfg.readFile(configPath);
    }
    catch(const libconfig::FileIOException &fioex)
    {
        std::cerr << "I/O error while reading file." << std::endl;
        return EXIT_FAILURE;
    }
    catch(const libconfig::ParseException &pex)
    {
        std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError() << std::endl;
        return EXIT_FAILURE;
    }

    float dt = cfg.lookup("dt");
    glm::vec2 G;
    G.x = cfg.lookup("gravity.x");
    G.y = cfg.lookup("gravity.y");

    const int num_iterations = cfg.lookup("num_iterations");
    const float K = cfg.lookup("K");
    const float h = cfg.lookup("h");
    const float h2 = h * h;
    const float h6 = pow(h, 0);
    const float p0 = cfg.lookup("p0");
    const float e = cfg.lookup("viscosity");

    const float poly6_coeff = 315.0f / (64.0f * PI * pow(h, 0));
    const float spiky_coeff = -45 / (PI * h6);
    const float viscosity_lap_coeff = 45 / (PI * h6);

    const float mass = cfg.lookup("mass");
    const float max_vel = cfg.lookup("max_vel");
    const float max_acc = cfg.lookup("max_acc");

    const float mouse_coeff = cfg.lookup("mouse_coeff");

    const int cellSize = 16;
    const int gridDimX = (width + 1) / cellSize;
    const int gridDimY = (height + 1) / cellSize;
    std::unordered_set<point*> grid[gridDimY][gridDimX];
    std::vector<point*> points;

    glm::ivec2 tl(0, 450);
    glm::ivec2 br(width, height-10);
    float radius = 4.0f;
    float dist = h - 0.0001f;

    generateParticles(points, grid, tl, br, dist, cellSize);

    tl = { 200, 100 };
    br = { 350, 175 };
    int cnt = 0;

    implicitEuler _integrator([=](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash + G;
    });

    Uint32 lastUpd = SDL_GetTicks();
    while(running) {
        Uint32 curTime = SDL_GetTicks();
        if(curTime - lastUpd >= 16) {
            handleInput(running, _mouse);
            if(_mouse->getRB() && cnt < 6) {
                generateParticles(points, grid, tl, br, h, cellSize);
                _mouse->setRB(false);
                cnt++;
            }

            _renderer->clearScreen(0xFF000816);

            for(int i = 0; i < num_iterations; i++) {
                // density and pressure
                for(int r = 0; r < gridDimY; r++)  for(int c = 0; c < gridDimX; c++) {
                    for(auto& p : grid[r][c]) {
                        // density
                        p->density = 0;
                        const int irmin = std::max(0, r - 1), irmax = std::min(gridDimY - 1, r + 1);
                        const int icmin = std::max(0, c - 1), icmax = std::min(gridDimX - 1, c + 1);
                        for(int ir = irmin; ir <= irmax; ir++) for(int ic = icmin; ic <= icmax; ic++) {
                            for(auto& q : grid[ir][ic]) {
                                const glm::vec2 diff = p->pos - q->pos;
                                const float r2 = glm::dot(diff, diff);
                                if(r2 < h2) {
                                    const float W = poly6_coeff * (h2 - r2) * (h2 - r2) * (h2 - r2);
                                    p->density += mass * W;
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

                // acceleration
                for(int r = 0; r < gridDimY; r++)  for(int c = 0; c < gridDimX; c++) {
                    for(auto& p : grid[r][c]) {
                        p->acc = { 0, 0 };
                        const int irmin = std::max(0, r - 1), irmax = std::min(gridDimY - 1, r + 1);
                        const int icmin = std::max(0, c - 1), icmax = std::min(gridDimX - 1, c + 1);
                        for(int ir = irmin; ir <= irmax; ir++) for(int ic = icmin; ic <= icmax; ic++) {
                            for(auto& q : grid[ir][ic]) {
                                if(q == p)
                                    continue;
                                const glm::vec2 diff = p->pos - q->pos;
                                const float r2 = glm::dot(diff, diff);
                                const float r = sqrt(r2);

                                if(r > 1e-3 && r < h) {
                                    const float W_spiky = spiky_coeff * (h - r) * (h - r);
                                    const float W_lap = viscosity_lap_coeff * (h - r);
                                    p->acc -= (mass / mass) * ((p->pressure + q->pressure) / (2.0f * p->density * q->density)) * W_spiky * (diff / r);
                                    p->acc += e * (mass / mass) * (1.0f / q->density) * (q->vel - p->vel) * W_lap;
                                }
                            }
                        }
                        if(p->pos.y >= height-10) {
                            const glm::vec2 diff = { 0, p->pos.y - height + 10 - h };
                            const float r2 = glm::dot(diff, diff);
                            const float r = sqrt(r2);
                            if(r > 1e-3 && r < h) {
                                const float W_spiky = spiky_coeff * (h - r) * (h - r);
                                p->acc -= p->pressure / (2.0f * p->density * p0) * W_spiky * (diff / r);
                            }
                        }
                        if(isnan(p->acc.x) || isnan(p->acc.y)) {
                            std::cout << "nan encountered in acc";
                            return 0;
                        }
                        // capMagnitude(p->acc, 0.5f);
                    }
                }

                // integrate movements
                for(auto& p : points) {
                    // _integrator.integrate(p->pos, p->vel, p->acc, dt);

                    // calculate velocity
                    if(_mouse->getLB()) {
                        glm::vec2 toMouse = _mouse->getPos() - p->pos;
                        if(glm::dot(toMouse, toMouse) < 32 * 32)
                            p->vel += mouse_coeff * _mouse->getDiff();
                    }
                    _integrator.integrateStep1(p->pos, p->vel, p->acc, dt);
                    capMagnitude(p->vel, max_vel);
                    
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
    cleanUtils();

    std::cout << "Quit program" << std::endl;

    return 0;
}