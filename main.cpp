#define SDL_MAIN_HANDLED
#include <iostream>
#include <SDL2/SDL.h>
#include <omp.h>
#include <libconfig.h++>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "ODE_solvers/implicitEuler.h"
#include "mouse.h"
#include "utils.h"
#include "fluid_sim.h"
#include "global.h"

const char* generalConfigPath = "config/general.cfg";
const char* utilsConfigPath = "config/utils.cfg";

int main() {
    const int width = 512, height = 512;

    libconfig::Config cfg;
    parseConfig(cfg, generalConfigPath);

    float dt = cfg.lookup("dt");
    glm::vec2 G;
    G.x = cfg.lookup("gravity.x");
    G.y = cfg.lookup("gravity.y");

    implicitEuler _integrator([=](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash + G;
    });

    fluid_sim* sim = new fluid_sim();
    sim->setup(cfg, width, height, &_integrator);
    sim->generateInitialParticles();

    Uint32 lastUpd = SDL_GetTicks();
    while(sim->isRunning()) {
        Uint32 curTime = SDL_GetTicks();
        if(curTime - lastUpd >= 16) {
            sim->input();

            try {
                sim->update();
            } catch(const char* err) {
                std::cout << err << std::endl;
                break;
            }

            sim->render();

            lastUpd = curTime;
        }
    }

    sim->destroy();
    cleanUtils();
    delete sim;

    std::cout << "Quit program" << std::endl;

    return 0;
}