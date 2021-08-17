cmake_minimum_required (VERSION 3.2)
project (sosage_emscripten)

## Use C++17
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../cmake")
include(Sosage_version)

## Fetch sources and create executable

file(GLOB_RECURSE SOSAGE_SRC "../../src/Sosage/*.cpp")

file(GLOB_RECURSE YAML_SRC "libyaml/src/*.c")
file(GLOB_RECURSE LZ4_SRC "lz4/lib/*.c")
add_executable(superflu-riteurnz ${SOSAGE_SRC} ${YAML_SRC} ${LZ4_SRC})
target_compile_options(superflu-riteurnz PUBLIC -DSOSAGE_DATA_FOLDER="data/")
target_compile_options(superflu-riteurnz PUBLIC -DSOSAGE_INSTALL_DATA_FOLDER="data/")
target_compile_definitions(superflu-riteurnz PUBLIC
  "-DSOSAGE_LINKED_WITH_SDL_MIXER")

## Compilation dependencies

target_include_directories (superflu-riteurnz PUBLIC
  "../../include"
  "libyaml/include"
  "libyaml/src"
  "lz4/lib")

# Yaml config
target_compile_definitions(superflu-riteurnz PUBLIC
  "-DYAML_VERSION_MAJOR=0")
target_compile_definitions(superflu-riteurnz PUBLIC
  "-DYAML_VERSION_MINOR=2")
target_compile_definitions(superflu-riteurnz PUBLIC
  "-DYAML_VERSION_PATCH=2")
target_compile_definitions(superflu-riteurnz PUBLIC
  "-DYAML_VERSION_STRING=\"0.2.2\"")

## Link dependencies

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file data")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_IMAGE=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SDL2_IMAGE_FORMATS='[\"png\"]'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_TTF=2")


target_link_libraries(superflu-riteurnz PUBLIC "-s WASM=1")
target_link_libraries(superflu-riteurnz PUBLIC "-o superflu-riteurnz.html")
