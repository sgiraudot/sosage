# Sosage pour Emscripten

## Compilation

1. Télécharger et installer Emscripten SDK
2. `$ cd emscripten`
3. `$ mkdir build && cd build`
4. `$ ln -s [where_the_data_is]/data data`
5. `$ ln -s [libyaml_source_path] libyaml`
6. `$ emcmake cmake .. -DCMAKE_BUILD_TYPE=Release`
7. `$ make`
