#include <iostream>
#include <stdexcept>
#include <sstream>
#include <unordered_set>
#include <unistd.h>
#include <libconfig.h++>
#include "utils.h"
#include "global.h"

static libconfig::Config utilsconfig;
static float bounceCoeff;
static float groundBounceCoeff;

bool getOption(int argc, char** argv, char opt) {
    static std::unordered_set<char> opt_set;
    static bool parsed = false;
    if(!parsed) {
        char c;
        while((c = getopt(argc, argv, argOpts)) != -1) {
            opt_set.insert(c);
        }
        parsed = true;
    }
    return opt_set.find(opt) != opt_set.end();
}

void getUtilsConfig() {
    parseConfig(utilsconfig, utilsConfigPath);
    bounceCoeff = utilsconfig.lookup("bounce_coeff");
    groundBounceCoeff = utilsconfig.lookup("ground_bounce_coeff");
}

void parseConfig(libconfig::Config& cfg, const char* configPath) {
    try {
        cfg.readFile(configPath);
    }
    catch(const libconfig::FileIOException &fioex)
    {
        throw std::runtime_error("I/O error while reading file.");
    }
    catch(const libconfig::ParseException &pex)
    {
        std::stringstream ss;
        ss << "Parse error at " << pex.getFile() << ":" << pex.getLine() << " - " << pex.getError();
        throw std::runtime_error(ss.str());
    }
}

void resolveOutOfBounds(point& p, int w, int h) {
    if(p.pos.x > w) {
        p.pos.x = w;
        p.vel.x *= -bounceCoeff;
    }
    if(p.pos.x < 0){
        p.pos.x = 0;
        p.vel.x *= -bounceCoeff;
    }
    if(p.pos.y > h - 10) {
        p.pos.y = h - 10;
        p.vel.y *= -bounceCoeff * groundBounceCoeff;
    }
    if(p.pos.y < 0) {
        p.pos.y = 0;
        p.vel.y *= -bounceCoeff;
    }
}

void resolveVelocity(const glm::vec2& p, glm::vec2& v, const int& height) {
    if(abs(v.x) < 0.25f)
        v.x = 0;
    if(abs(v.y) < 0.25f && p.y > height - 10 - 3)
        v.y = 0;
    float len = glm::length(v);
    v *= len > 100.0f ? 100.0f / len : 1;
}