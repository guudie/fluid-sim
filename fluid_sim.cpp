#include <stdexcept>
#include "fluid_sim.h"
#include "renderer.h"
#include "mouse.h"
#include "ODE_solvers/ODESolver.h"
#include "utils.h"
#include "glm/glm.hpp"

#define PI 3.14159265359
#define EPS 1e-6

/*
    NONE,
    NAN_DENSITY,
    NAN_PRESSURE,
    NAN_ACC,
    NAN_POS,
    IDX_OUT_OF_RANGE
*/
const char* getErrorMessage(const parallel_exception& err) {
    switch(err) {
    case parallel_exception::NAN_DENSITY:
        return "Nan encountered in density";
        break;
    case parallel_exception::NAN_PRESSURE:
        return "Nan encountered in pressure";
        break;
    case parallel_exception::NAN_ACC:
        return "Nan encountered in acc";
        break;
    case parallel_exception::NAN_POS:
        return "Nan encountered in position";
        break;
    case parallel_exception::IDX_OUT_OF_RANGE:
        return "Index out of range";
        break;
    default:
        return "No error";
        break;
    }
}

static void capMagnitude(glm::vec2& v, float maxMag) {
    float len = glm::length(v);
    if(len < EPS) {
        v = { 0, 0 };
        return;
    }
    if(len > maxMag)
        v *= maxMag / len;
}

void fluid_sim::setup(const libconfig::Config& cfg, int windowWidth, int windowHeight, ODESolver* integrator) {
    assert(integrator != nullptr);
    _integrator = integrator;
    _renderer = new renderer();
    _mouse = new mouse();

    tickDuration = cfg.lookup("tick_duration");
    num_iterations = cfg.lookup("num_iterations");
    K = cfg.lookup("K");
    h = cfg.lookup("h");
    h2 = h * h;
    h6 = pow(h, 0);
    p0 = cfg.lookup("p0");
    e = cfg.lookup("viscosity");

    poly6_coeff = 315.0f / (64.0f * PI * pow(h, 0));
    spiky_coeff = -45 / (PI * h6);
    viscosity_lap_coeff = 45 / (PI * h6);

    mass = cfg.lookup("mass");
    max_vel = cfg.lookup("max_vel");
    max_acc = cfg.lookup("max_acc");

    mouse_coeff = cfg.lookup("mouse_coeff");

    cellSize = cfg.lookup("cell_size");
    gridDimX = windowWidth / cellSize;
    gridDimY = windowHeight / cellSize;

    grid = new std::unordered_set<point*>*[gridDimY];
    gridLock = new omp_lock_t*[gridDimY];
    for(int i = 0; i < gridDimY; i++) {
        grid[i] = new std::unordered_set<point*>[gridDimX];
        gridLock[i] = new omp_lock_t[gridDimX];
        for(int j = 0; j < gridDimX; j++)
            omp_init_lock(&gridLock[i][j]);
    }

    running = _renderer->setup(windowWidth, windowHeight);

    lastUpdateTime = SDL_GetTicks();
}

bool fluid_sim::checkShouldUpdate() {
    currentTime = SDL_GetTicks();
    if(currentTime - lastUpdateTime >= tickDuration) {
        lastUpdateTime = currentTime;
        return true;
    }
    return false;
}

void fluid_sim::input() {
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

void fluid_sim::postInput() {
    if(_mouse->getRB() && generateCount < 6) {
        const int genWidth = 150;
        glm::ivec2 tl = { (_renderer->getWidth() - genWidth) / 2, 100 };
        glm::ivec2 br = { tl.x + genWidth, 175 };
        generateParticles(tl, br, h);
        _mouse->setRB(false);
        generateCount++;
    }
}

void fluid_sim::generateInitialParticles() {
    glm::ivec2 tl(0, 450);
    glm::ivec2 br(_renderer->getWidth()-1, _renderer->getHeight()-11);
    generateParticles(tl, br, h - 0.0001f);
}

void fluid_sim::generateParticles(const glm::ivec2& from, const glm::ivec2& to, float dist) {
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

void fluid_sim::calcDensityAndPressure() {
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
                throw std::runtime_error("Nan encountered in density");
            }
            p->density = std::max(p0, p->density);

            // pressure
            p->pressure = K * (p->density - p0);
            if(isnan(p->pressure)) {
                throw std::runtime_error("Nan encountered in pressure");
            }
        }
    }
}

void fluid_sim::calcAcceleration() {
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

                    if(r > EPS && r < h) {
                        const float W_spiky = spiky_coeff * (h - r) * (h - r);
                        const float W_lap = viscosity_lap_coeff * (h - r);
                        p->acc -= (mass / mass) * ((p->pressure + q->pressure) / (2.0f * p->density * q->density)) * W_spiky * (diff / r);
                        p->acc += e * (mass / mass) * (1.0f / q->density) * (q->vel - p->vel) * W_lap;
                    }
                }
            }
            if(p->pos.y >= _renderer->getHeight()-11) {
                const glm::vec2 diff = { 0, p->pos.y - _renderer->getHeight() + 11 - h };
                const float r2 = glm::dot(diff, diff);
                const float r = sqrt(r2);
                if(r > EPS && r < h) {
                    const float W_spiky = spiky_coeff * (h - r) * (h - r);
                    p->acc -= p->pressure / (2.0f * p->density * p0) * W_spiky * (diff / r);
                }
            }
            if(isnan(p->acc.x) || isnan(p->acc.y)) {
                throw std::runtime_error("Nan encountered in acc");
            }
            // capMagnitude(p->acc, 0.5f);
        }
    }
}

void fluid_sim::integrateMovements() {
    for(auto& p : points) {
        // _integrator.integrate(p->pos, p->vel, p->acc, dt);

        // calculate velocity
        if(_mouse->getLB()) {
            glm::vec2 toMouse = _mouse->getPos() - p->pos;
            if(glm::dot(toMouse, toMouse) < 32 * 32)
                p->vel += mouse_coeff * _mouse->getDiff();
        }
        _integrator->integrateStep1(p->pos, p->vel, p->acc, dt);
        capMagnitude(p->vel, max_vel);
        
        _integrator->integrateStep2(p->pos, p->vel, dt);
        resolveOutOfBounds(*p, _renderer->getWidth()-1, _renderer->getHeight()-1);

        if(isnan(p->pos.x) || isnan(p->pos.y)) {
            throw std::runtime_error("Nan encountered in position");
        }
        glm::ivec2 newIdx = { p->pos.x / cellSize, p->pos.y / cellSize };
        if(p->gridIdx != newIdx) {
            if(newIdx.x < 0 || newIdx.x >= gridDimX || newIdx.y < 0 || newIdx.y >= gridDimY) {
                throw std::runtime_error("Index out of range");
            }
            grid[p->gridIdx.y][p->gridIdx.x].erase(p);
            grid[newIdx.y][newIdx.x].insert(p);
            p->gridIdx = newIdx;
        }
    }
}

void fluid_sim::calcDensityAndPressureParallel() {
    #pragma omp parallel for collapse(2)
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
            #pragma omp critical(check_nan_density)
            {
                if(par_excpt == parallel_exception::NONE && isnan(p->density)) {
                    par_excpt = parallel_exception::NAN_DENSITY;
                }
            }
            p->density = std::max(p0, p->density);

            // pressure
            p->pressure = K * (p->density - p0);
            #pragma omp critical(check_nan_pressure)
            {
                if(par_excpt == parallel_exception::NONE && isnan(p->pressure)) {
                    par_excpt = parallel_exception::NAN_PRESSURE;
                }
            }
        }
    }
}

void fluid_sim::calcAccelerationParallel() {
    #pragma omp parallel for collapse(2)
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

                    if(r > EPS && r < h) {
                        const float W_spiky = spiky_coeff * (h - r) * (h - r);
                        const float W_lap = viscosity_lap_coeff * (h - r);
                        p->acc -= (mass / mass) * ((p->pressure + q->pressure) / (2.0f * p->density * q->density)) * W_spiky * (diff / r);
                        p->acc += e * (mass / mass) * (1.0f / q->density) * (q->vel - p->vel) * W_lap;
                    }
                }
            }
            if(p->pos.y >= _renderer->getHeight()-11) {
                const glm::vec2 diff = { 0, p->pos.y - _renderer->getHeight() + 11 - h };
                const float r2 = glm::dot(diff, diff);
                const float r = sqrt(r2);
                if(r > EPS && r < h) {
                    const float W_spiky = spiky_coeff * (h - r) * (h - r);
                    p->acc -= p->pressure / (2.0f * p->density * p0) * W_spiky * (diff / r);
                }
            }
            #pragma omp critical(check_nan_acc)
            {
                if(par_excpt == parallel_exception::NONE && (isnan(p->acc.x) || isnan(p->acc.y))) {
                    par_excpt = parallel_exception::NAN_ACC;
                }
            }
            // capMagnitude(p->acc, 0.5f);
        }
    }
}

void fluid_sim::integrateMovementsParallel() {
    #pragma omp parallel for
    for(auto& p : points) {
        // _integrator.integrate(p->pos, p->vel, p->acc, dt);

        // calculate velocity
        if(_mouse->getLB()) {
            glm::vec2 toMouse = _mouse->getPos() - p->pos;
            if(glm::dot(toMouse, toMouse) < 32 * 32)
                p->vel += mouse_coeff * _mouse->getDiff();
        }
        _integrator->integrateStep1(p->pos, p->vel, p->acc, dt);
        capMagnitude(p->vel, max_vel);
        
        _integrator->integrateStep2(p->pos, p->vel, dt);
        resolveOutOfBounds(*p, _renderer->getWidth()-1, _renderer->getHeight()-1);

        #pragma omp critical(check_nan_pos)
        {
            if(par_excpt == parallel_exception::NONE && (isnan(p->pos.x) || isnan(p->pos.y))) {
                par_excpt = parallel_exception::NAN_POS;
            }
        }
        glm::ivec2 newIdx = { p->pos.x / cellSize, p->pos.y / cellSize };
        if(p->gridIdx != newIdx) {
            if(newIdx.x < 0 || newIdx.x >= gridDimX || newIdx.y < 0 || newIdx.y >= gridDimY) {
                #pragma omp critical(check_idx_oor)
                {
                    if(par_excpt == parallel_exception::NONE) {
                        par_excpt = parallel_exception::IDX_OUT_OF_RANGE;
                    }
                }
            } else {
                // erase p from grid[p->gridIdx.y][p->gridIdx.x]
                omp_set_lock(&gridLock[p->gridIdx.y][p->gridIdx.x]);
                grid[p->gridIdx.y][p->gridIdx.x].erase(p);
                omp_unset_lock(&gridLock[p->gridIdx.y][p->gridIdx.x]);

                // insert p into grid[newIdx.y][newIdx.x]
                omp_set_lock(&gridLock[newIdx.y][newIdx.x]);
                grid[newIdx.y][newIdx.x].insert(p);
                omp_unset_lock(&gridLock[newIdx.y][newIdx.x]);

                // update grid index of p
                p->gridIdx = newIdx;
            }
        }
    }
}

void fluid_sim::update() {
    for(int i = 0; i < num_iterations; i++) {
        calcDensityAndPressure();
        calcAcceleration();
        integrateMovements();
    }
}

void fluid_sim::updateParallel() {
    for(int i = 0; i < num_iterations; i++) {
        calcDensityAndPressureParallel();
        if(par_excpt != parallel_exception::NONE) {
            throw std::runtime_error(getErrorMessage(par_excpt));
        }

        calcAccelerationParallel();
        if(par_excpt != parallel_exception::NONE) {
            throw std::runtime_error(getErrorMessage(par_excpt));
        }

        integrateMovementsParallel();
        if(par_excpt != parallel_exception::NONE) {
            throw std::runtime_error(getErrorMessage(par_excpt));
        }
    }
}

void fluid_sim::render() {
    _renderer->clearScreen(0xFF000816);

    for(auto& p : points)
        _renderer->drawCircle(p->pos, radius, 0xFF55AADD);

    _renderer->render();
}

bool fluid_sim::isRunning() const {
    return running;
}

Uint32 fluid_sim::getTickDuration() const {
    return tickDuration;
}

mouse* const& fluid_sim::getMouseObject() const {
    return _mouse;
}

float fluid_sim::getRadius() const {
    return radius;
}

float fluid_sim::getH() const {
    return h;
}

void fluid_sim::destroy() {
    for(int i = 0; i < gridDimY; i++) for(int j = 0; j < gridDimX; j++)
        omp_destroy_lock(&gridLock[i][j]);
    
    for(int i = 0; i < gridDimY; i++) {
        delete[] grid[i];
        delete[] gridLock[i];
    }
    delete[] grid;
    delete[] gridLock;

    for(auto& p : points)
        delete p;
    
    delete _mouse;
    delete _renderer;
}