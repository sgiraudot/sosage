cmake_minimum_required (VERSION 3.2)
project (superflu_pnc)

## Use C++17 and load local CMake modules
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

## Check all dependencies

set(SUPERFLU_PNC_DEPENDENCIES_OKAY true)

find_package(YAML REQUIRED)
if (NOT YAML_FOUND)
  message(STATUS "libyaml is required and not found")
  set(SUPERFLU_PNC_DEPENDENCIES_OKAY false)
endif()

find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
  message(STATUS "SDL2 is required and not found")
  set(SUPERFLU_PNC_DEPENDENCIES_OKAY false)
endif()  

find_package(SDL2_mixer QUIET)
if (NOT SDL2_MIXER_FOUND)
  message(STATUS "SDL2 Mixer is optional and not found (no sound will be available)")
endif()  

find_package(SDL2_image REQUIRED)
if (NOT SDL2_IMAGE_FOUND)
  message(STATUS "SDL2 Image is required and not found")
  set(SUPERFLU_PNC_DEPENDENCIES_OKAY false)
endif()  

find_package(SDL2_ttf REQUIRED)
if (NOT SDL2_ttf_FOUND)
  message(STATUS "SDL2 TTF is required and not found")
  set(SUPERFLU_PNC_DEPENDENCIES_OKAY false)
endif()

## Abort if at least one dependency is missing

if (NOT SUPERFLU_PNC_DEPENDENCIES_OKAY)
  message(STATUS "Some dependencies are missing (see below), project won't be compiled")
  return()
endif()

## Fetch sources and create executable

file(GLOB_RECURSE SUPERFLU_PNC_SRC "src/Sosage/*.cpp")
add_executable(superflu_pnc "src/main.cpp" ${SUPERFLU_PNC_SRC})

## Set up data folder

target_compile_options(superflu_pnc PUBLIC -DSUPERFLU_DATA_FOLDER="${CMAKE_CURRENT_LIST_DIR}/data/")

## Compilation dependencies

target_include_directories (superflu_pnc PUBLIC
  "include"
  ${YAML_INCLUDE_DIR}
  ${SDL2_INCLUDE_DIR}
  ${SDL2_IMAGE_INCLUDE_DIR}
  ${SDL2_TTF_INCLUDE_DIR})

## Link dependencies

message(${YAML_LIBRARIES})
message(${SDL2_LIBRARY})

target_link_libraries(superflu_pnc PUBLIC
  ${YAML_LIBRARIES}
  ${SDL2_LIBRARY}
  ${SDL2_IMAGE_LIBRARIES}
  ${SDL2_TTF_LIBRARIES})

## Optional sound
if (SDL2_MIXER_FOUND)
  target_include_directories (superflu_pnc PUBLIC ${SDL2_MIXER_INCLUDE_DIR})
  target_link_libraries(superflu_pnc PUBLIC ${SDL2_MIXER_LIBRARIES})
  target_compile_definitions(superflu_pnc PUBLIC "-DSOSAGE_LINKED_WITH_SDL_MIXER")
endif()

## Platform-specific instructions
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  target_compile_options(superflu_pnc PUBLIC -DSUPERFLU_INSTALL_DATA_FOLDER="../share/games/superflu_pnc/")
  install(TARGETS superflu_pnc DESTINATION games)
  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data/ DESTINATION share/games/superflu_pnc)
  install(FILES ${CMAKE_CURRENT_LIST_DIR}/resources/icon.svg DESTINATION share/icons/hicolor/scalable/apps/ RENAME superflu_pnc.svg)
  install(FILES ${CMAKE_CURRENT_LIST_DIR}/resources/superflu_pnc.desktop DESTINATION share/applications/)
elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
  target_compile_options(superflu_pnc PUBLIC -DSUPERFLU_INSTALL_DATA_FOLDER="data/")
  install(TARGETS superflu_pnc DESTINATION Superflu\ PnC)
  install(DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/data/ DESTINATION Superflu\ PnC/data)
elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
  
endif()

# Activate concurrency?
option(SOSAGE_COMPILE_TESTS "Compile Sosage tests" OFF)

if(SOSAGE_COMPILE_TESTS)

  enable_testing()
  
  set (SOSAGE_TEST_TARGETS "unit_component_action")

  foreach(target ${SOSAGE_TEST_TARGETS})
    add_executable("test_${target}" "test/${target}.cpp" ${SUPERFLU_PNC_SRC})
    target_include_directories ("test_${target}" PUBLIC
      "include"
      ${YAML_INCLUDE_DIR}
      ${SDL2_INCLUDE_DIR}
      ${SDL2_IMAGE_INCLUDE_DIR}
      ${SDL2_TTF_INCLUDE_DIR})
    target_link_libraries("test_${target}" PUBLIC
      ${YAML_LIBRARIES}
      ${SDL2_LIBRARY}
      ${SDL2_IMAGE_LIBRARIES}
      ${SDL2_TTF_LIBRARIES})
    
    if (SDL2_MIXER_FOUND)
      target_include_directories ("test_${target}" PUBLIC ${SDL2_MIXER_INCLUDE_DIR})
      target_link_libraries("test_${target}" PUBLIC ${SDL2_MIXER_LIBRARIES})
      target_compile_definitions("test_${target}" PUBLIC "-DSOSAGE_LINKED_WITH_SDL_MIXER")
    endif()

    add_test(${target} "test_${target}" "")
  endforeach()
  
endif()
