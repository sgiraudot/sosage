# Sosage pour Emscripten

## Compilation

1. Télécharger et installer Emscripten SDK
2. `$ cd emscripten`
3. `$ ln -s [libyaml_source_path] libyaml`
3. `$ ln -s [liblz4_source_path] lz4`
4. `$ mkdir build && cd build`
5. `$ ln -s [where_the_data_is]/data data`
6. `$ emcmake cmake .. -DCMAKE_BUILD_TYPE=Release`
7. `$ make`
