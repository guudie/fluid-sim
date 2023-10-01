### Requirements
For windows users, you can skip this section (**Requirements**) and the next one (**Build and run**) and just unzip the `application.zip` file if you just want a working executable. Otherwise, if you prefer to build it yourself, here are the requirements:
* GNU make
* Python 3.x (to generate test data)
* MinGW64 (for Windows, [msys2.org](https://www.msys2.org/))
* `g++` compiler with the following libraries installed
  * `SDL2` ([libsdl.org](https://www.libsdl.org/)), which provides rendering functionality
  * `libconfig` ([hyperrealm.github.io](https://hyperrealm.github.io/libconfig/)), which provides config file parsing

### Build and run
With GNU make installed, open cmd/terminal in the `water-sim/` folder and run the command `make`, the output executable with be named `app`. Note that you may need some additional dynamic/shared libraries installed system wide in order to compile and run the app properly.

Run `app`.

### Prebuilt executable (for windows)
If building the program yourself is not an option, you can unzip `application.zip`, which contains the executable itself (`app.exe`). Similar to compiling the program yourself, you may need some dynamic libraries installed system wide which, hopefully, already came with the operating system. Otherwise, you can download any missing `.dll`s from a Google search.

### Interactions
Move the particles with the cursor by holding left mouse button. Add more particles by clicking right mouse button (this can be done at most 6 times).

The parameters for fluid dynamics are defined in `config/` folder, you can tweak them if you know what you're doing.