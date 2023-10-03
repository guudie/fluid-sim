#define SDL_MAIN_HANDLED
#include <iostream>
#include <stdexcept>
#include <SDL2/SDL.h>
#include <omp.h>
#include <libconfig.h++>
#include "glm/glm.hpp"
#include "glm/gtx/string_cast.hpp"
#include "ODE_solvers/implicitEuler.h"
#include "utils.h"
#include "fluid_sim.h"
#include "global.h"

const char* argOpts = "m";
const char* generalConfigPath = "config/general.cfg";
const char* utilsConfigPath = "config/utils.cfg";

int main(int argc, char** argv) {
    const int width = 512, height = 512;
    bool multithread = getOption(argc, argv, 'm');

    if(multithread)
        std::cout << "Number of parallel threads: " << omp_get_max_threads() << std::endl;

    libconfig::Config cfg;
    try {
        parseConfig(cfg, generalConfigPath);
        getUtilsConfig();
    } catch(std::runtime_error& rex) {
        std::cout << rex.what() << std::endl;
        return EXIT_FAILURE;
    }

    glm::vec2 G;
    G.x = cfg.lookup("gravity.x");
    G.y = cfg.lookup("gravity.y");

    implicitEuler _integrator([=](float t, glm::vec2 y, glm::vec2 z, glm::vec2 zdash) -> glm::vec2 {
        return zdash + G;
    });

    fluid_sim* sim = new fluid_sim();
    sim->setup(cfg, width, height, (ODESolver*)&_integrator);
    sim->generateInitialParticles();

    while(sim->isRunning()) {
        if(sim->checkShouldUpdate()) {
            sim->input();
            sim->postInput();

            try {
                if(multithread)
                    sim->updateMultithread();
                else
                    sim->update();
            } catch(std::runtime_error& rex) {
                std::cout << rex.what() << std::endl;
                break;
            }

            sim->render();
        }
    }

    sim->destroy();
    delete sim;

    std::cout << "Quit program" << std::endl;

    return 0;
}