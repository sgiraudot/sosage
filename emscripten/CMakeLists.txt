cmake_minimum_required (VERSION 3.2)
project (sosage_emscripten)

## Use C++17
set(CMAKE_CXX_STANDARD 17)

## Set up data folder

## Fetch sources and create executable

file(GLOB_RECURSE SOSAGE_SRC "../src/Sosage/*.cpp")
file(GLOB_RECURSE YAML_SRC "libyaml/src/*.c")
add_executable(superflu_pnc ${SOSAGE_SRC} ${YAML_SRC})
target_compile_options(superflu_pnc PUBLIC -DSOSAGE_DATA_FOLDER="data/")
target_compile_definitions(superflu_pnc PUBLIC
  "-DSOSAGE_LINKED_WITH_SDL_MIXER")

## Compilation dependencies

target_include_directories (superflu_pnc PUBLIC
  "../include"
  "libyaml/include"
  "libyaml/src")

# Yaml config
target_compile_definitions(superflu_pnc PUBLIC
  "-DYAML_VERSION_MAJOR=0")
target_compile_definitions(superflu_pnc PUBLIC
  "-DYAML_VERSION_MINOR=2")
target_compile_definitions(superflu_pnc PUBLIC
  "-DYAML_VERSION_PATCH=2")
target_compile_definitions(superflu_pnc PUBLIC
  "-DYAML_VERSION_STRING=\"0.2.2\"")

## Link dependencies

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --embed-file data")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s TOTAL_MEMORY=268435456")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_IMAGE=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SDL2_IMAGE_FORMATS='[\"png\"]'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_TTF=2")


target_link_libraries(superflu_pnc PUBLIC "-s WASM=1")
# target_link_libraries(superflu_pnc PUBLIC "-s USE_SDL=2")
# target_link_libraries(superflu_pnc PUBLIC "-s USE_SDL_IMAGE=2")
# target_link_libraries(superflu_pnc PUBLIC "-s USE_SDL_MIXER=2")
# target_link_libraries(superflu_pnc PUBLIC "-s USE_SDL_TTF=2")
target_link_libraries(superflu_pnc PUBLIC "-o superflu_pnc.html")
