# Sosage (Superfluous Open Source Adventure Game Engine)

***Note:** this repository is just a mirror of [the official
repository on Framagit](https://framagit.org/Gee/sosage), which should
be considered as the most up-to-date and reliable source for Sosage.*

## Dependencies

 - SDL2 (>= 2.0.10)
 - SDL2_Image
 - SDL2_TTF
 - SDL2_Mixer
 - libyaml
 - liblz4

## Game Data

This repository only contains the source code of the Sosage engine:
game data are provided separately. To compile the game, you need to
specify the folder containing game data through the cmake variable
`SOSAGE_DATA_FOLDER`. That folder must contain the provided
`config.cmake` file.

## Compilation (Debian-based GNU/Linux)

For Debian-based GNU/Linux distributions:

1. `# apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libyaml-dev liblz4-1`
2. `$ mkdir build && cd build`
3. `$ cmake .. -DCMAKE_BUILD_TYPE=Release -DSOSAGE_DATA_FOLDER=[where_the_data_is]`
4. `$ make`

Compilation should also work with other Linux distributions, with
Windows and with MacOS

For other platforms (Android, Emscripten, etc.), please refer to the
`README.md` files in the subfolders of `platform`.
