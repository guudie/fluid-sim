#include "application.h"
#include "renderer.h"
#include "mouse.h"
#include "ODE_solvers/ODESolver.h"

void application::setupFromPath(int windowWidth, int windowHeight, std::string path, ODESolver* _integrator) {
    _renderer = new renderer();
    _mouse = new mouse();
    _cloth = new cloth(_integrator);

    running = _renderer->setup(windowWidth, windowHeight);

    // only allowing initialization from data path for now
    _cloth->initFromFile(path);

    lastUpdateTime = SDL_GetTicks();
}

void application::input() {
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

void application::updateNoTick() {
    currentTime = SDL_GetTicks();
    float dt = (currentTime - lastUpdateTime) * 60.0f / 1000.0f;    // dt is the ratio of actual deltatime to 16.6ms (60 FPS)

    _cloth->update(_mouse, _renderer->getWidth(), _renderer->getHeight(), dt);
    
    lastUpdateTime = currentTime;
}

void application::updateWithTick() {
    currentTime = SDL_GetTicks();
    if(currentTime - lastUpdateTime >= tickDuration) {
        _cloth->update(_mouse, _renderer->getWidth(), _renderer->getHeight());
    
        lastUpdateTime = currentTime;
        updatedThisTick = true;
    }
}

void application::update() {
    if(updateEveryTick)
        updateWithTick();
    else
        updateNoTick();
}

void application::render() {
    if(updateEveryTick && !updatedThisTick)
        return;
    _renderer->clearScreen(0xFF000816);

    _cloth->drawAllSticks(_renderer);

    _renderer->render();
    updatedThisTick = false;
}

bool application::isRunning() const {
    return running;
}

bool application::isUpdateEveryTick() const {
    return updateEveryTick;
}

void application::setTickUpdate(bool _updateEveryTick, Uint32 _tickDuration) {
    updateEveryTick = _updateEveryTick;
    tickDuration = _tickDuration;
}

mouse* const& application::getMouseObject() const {
    return _mouse;
}

void application::destroy() {
    delete _mouse;
    delete _renderer;
    delete _cloth;
}