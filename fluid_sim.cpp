#include "fluid_sim.h"
#include "renderer.h"
#include "mouse.h"
#include "ODE_solvers/ODESolver.h"
#include "utils.h"

void fluid_sim::setup(const libconfig::Config& cfg, int windowWidth, int windowHeight, ODESolver* integrator) {
    assert(integrator != nullptr);
    _integrator = integrator;
    _renderer = new renderer();
    _mouse = new mouse();

    int num_iterations = cfg.lookup("num_iterations");
    float K = cfg.lookup("K");
    float h = cfg.lookup("h");
    float h2 = h * h;
    float h6 = pow(h, 0);
    float p0 = cfg.lookup("p0");
    float e = cfg.lookup("viscosity");
    float poly6_coeff = 315.0f / (64.0f * PI * pow(h, 0));
    float spiky_coeff = -45 / (PI * h6);
    float viscosity_lap_coeff = 45 / (PI * h6);
    float mass = cfg.lookup("mass");
    float max_vel = cfg.lookup("max_vel");
    float max_acc = cfg.lookup("max_acc");
    float mouse_coeff = cfg.lookup("mouse_coeff");
    int cellSize = 16;
    int gridDimX = (windowWidth + 1) / cellSize;
    int gridDimY = (windowHeight + 1) / cellSize;

    running = _renderer->setup(windowWidth, windowHeight);

    lastUpdateTime = SDL_GetTicks();
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

void fluid_sim::updateNoTick() {
    currentTime = SDL_GetTicks();
    float dt = (currentTime - lastUpdateTime) * 60.0f / 1000.0f;    // dt is the ratio of actual deltatime to 16.6ms (60 FPS)

    _cloth->update(_mouse, _renderer->getWidth(), _renderer->getHeight(), dt);
    
    lastUpdateTime = currentTime;
}

void fluid_sim::updateWithTick() {
    currentTime = SDL_GetTicks();
    if(currentTime - lastUpdateTime >= tickDuration) {
        _cloth->update(_mouse, _renderer->getWidth(), _renderer->getHeight());
    
        lastUpdateTime = currentTime;
        updatedThisTick = true;
    }
}

void fluid_sim::update() {
    if(updateEveryTick)
        updateWithTick();
    else
        updateNoTick();
}

void fluid_sim::render() {
    if(updateEveryTick && !updatedThisTick)
        return;
    _renderer->clearScreen(0xFF000816);

    _cloth->drawAllSticks(_renderer);

    _renderer->render();
    updatedThisTick = false;
}

bool fluid_sim::isRunning() const {
    return running;
}

bool fluid_sim::isUpdateEveryTick() const {
    return updateEveryTick;
}

void fluid_sim::setTickUpdate(bool _updateEveryTick, Uint32 _tickDuration) {
    updateEveryTick = _updateEveryTick;
    tickDuration = _tickDuration;
}

mouse* const& fluid_sim::getMouseObject() const {
    return _mouse;
}

void fluid_sim::destroy() {
    delete _mouse;
    delete _renderer;
    delete _cloth;
}