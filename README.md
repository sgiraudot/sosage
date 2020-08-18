# Sosage (Superfluous Open Source Adventure Game Engine)

## Dépendances

 - SDL2
 - SDL2_Image
 - SDL2_TTF
 - SDL2_Mixer
 - libyaml

## Compilation (GNU/Linux)

1. `# apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libyaml-dev`
2. `$ mkdir build && cd build`
3. `$ cmake .. -DCMAKE_BUILD_TYPE=Release -DSOSAGE_DATA_FOLDER=[where_the_data_is]`
4. `$ make`

## Compilation (Android)

1. Télécharger et installer SDK+NDK Android
2. `$ cd android`
3. `$ ln -s [sdl_source_path] app/jni/SDL`
4. `$ ln -s [sdl_image_source_path] app/jni/SDL_image`
5. `$ ln -s [sdl_mixer_source_path] app/jni/SDL_mixer`
6. `$ ln -s [sdl_ttf_source_path] app/jni/SDL_ttf`
7. `$ ln -s [libyaml_source_path] app/jni/yaml/libyaml`
8. `$ ln -s [where_the_data_is]/data app/src/main/assets/data`
8. `$ ./gradlew assemble`

## Compilation (Emscripten/navigateur)

1. Télécharger et installer Emscripten SDK
2. `$ cd emscripten`
3. `$ mkdir build`
4. `$ cd build`
5. `$ ln -s [where_the_data_is]/data data`
6. `$ ln -s [libyaml_source_path] libyaml`
7. `$ emcmake cmake .. -DCMAKE_BUILD_TYPE=Release`
8. `$ make`
