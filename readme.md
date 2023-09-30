### Requirements
* `GNU make`
* `g++` compiler with the following libraries installed
  * `SDL2` ([libsdl.org](https://www.libsdl.org/)), which provides rendering functionality
  * `libconfig` ([hyperrealm.github.io](https://hyperrealm.github.io/libconfig/)), which provides config file parsing

### Build and run
With `GNU make` installed, open cmd/terminal in the `water-sim/` folder and run the command `make`, the output executable with be named `app`.

Run `app`.

### Interactions
Move the particles with the cursor by holding left mouse button. Add more particles by clicking right mouse button (this can be done at most 6 times).

The parameters for fluid dynamics are defined in `config/` folder, you can tweak them if you know what you're doing.