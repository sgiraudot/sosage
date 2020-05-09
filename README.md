# Superflu PnC

## Dépendances

 - SDL2
 - SDL2_Image
 - SDL2_TTF
 - SDL2_Mixer
 - libyaml

## Compilation (GNU/Linux)

1. `# apt install libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev libyaml-dev`
2. `$ mkdir build && cd build`
3. `$ cmake .. -DCMAKE_BUILD_TYPE=Release`
4. `$ make`

## Compilation (Android)

1. Télécharger SDK+NDK Android
2. `$ cd android`
3. `$ ln -s [sdl_source_path] app/jni/SDL`
4. `$ ln -s [sdl_image_source_path] app/jni/SDL_image`
5. `$ ln -s [sdl_mixer_source_path] app/jni/SDL_mixer`
6. `$ ln -s [sdl_ttf_source_path] app/jni/SDL_ttf`
7. `$ ln -s [libyaml_source_path] app/jni/yaml/libyaml`
8. `$ ln -s ../share/superflu_pnc app/src/main/assets/superflu_pnc`
8. `$ ./gradlew assemble`

## Compilation (Navigateur)

