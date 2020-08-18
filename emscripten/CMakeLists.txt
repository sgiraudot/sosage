cmake_minimum_required (VERSION 3.2)
project (sosage_emscripten)

## Use C++17
set(CMAKE_CXX_STANDARD 17)

## Set up data folder

set(SOSAGE_DATA_FOLDER "" CACHE PATH "Where the game data is")

if (SOSAGE_DATA_FOLDER STREQUAL "")
  message(STATUS "No data folder provided, nothing will be compiled")
  return()
endif()

include(${SOSAGE_DATA_FOLDER}/config.cmake)

## Fetch sources and create executable

file(GLOB_RECURSE SOSAGE_SRC "../src/Sosage/*.cpp")
file(GLOB_RECURSE YAML_SRC "libyaml/src/*.c")
add_executable(${SOSAGE_EXE_NAME} ${SOSAGE_SRC} ${YAML_SRC})
target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_DATA_FOLDER="${SOSAGE_DATA_FOLDER}/data/")
target_compile_definitions(${SOSAGE_EXE_NAME} PUBLIC
  "-DSOSAGE_LINKED_WITH_SDL_MIXER")

## Compilation dependencies

target_include_directories (${SOSAGE_EXE_NAME} PUBLIC
  "../include"
  "libyaml/include"
  "libyaml/src")

# Yaml config
target_compile_definitions(${SOSAGE_EXE_NAME} PUBLIC
  "-DYAML_VERSION_MAJOR=0")
target_compile_definitions(${SOSAGE_EXE_NAME} PUBLIC
  "-DYAML_VERSION_MINOR=2")
target_compile_definitions(${SOSAGE_EXE_NAME} PUBLIC
  "-DYAML_VERSION_PATCH=2")
target_compile_definitions(${SOSAGE_EXE_NAME} PUBLIC
  "-DYAML_VERSION_STRING=\"0.2.2\"")

## Link dependencies

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_IMAGE=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SDL2_IMAGE_FORMATS='[\"png\"]'")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_TTF=2")


target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-s WASM=1")
# target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-s USE_SDL=2")
# target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-s USE_SDL_IMAGE=2")
# target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-s USE_SDL_MIXER=2")
# target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-s USE_SDL_TTF=2")
target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-o ${SOSAGE_EXE_NAME}.html")
