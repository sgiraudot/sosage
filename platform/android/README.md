# Sosage pour Android

## Compilation

1. Télécharger et installer SDK+NDK Android
2. `$ cd android`
3. `$ ln -s [sdl_source_path] app/jni/SDL`
4. `$ ln -s [sdl_image_source_path] app/jni/SDL_image`
5. `$ ln -s [sdl_mixer_source_path] app/jni/SDL_mixer`
6. `$ ln -s [sdl_ttf_source_path] app/jni/SDL_ttf`
7. `$ ln -s [libyaml_source_path] app/jni/yaml/libyaml`
8. `$ ln -s [where_the_data_is]/data app/src/main/assets/data`
9. `$ ./gradlew assemble`
