cmake_minimum_required (VERSION 3.2)
project (sosage)

## Use C++17 and load local CMake modules
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

## OPTIONS
function(sosage_option name description value)
  option(${name} ${description} ${value})
  if(${name})
    add_definitions(-D${name})
  endif()
endfunction()

sosage_option(SOSAGE_CFG_DISPLAY_DEBUG_INFO "Force displaying debug info even in Release mode" OFF)
sosage_option(SOSAGE_CFG_PROFILE "Profile and display timing info (requires debug info ON)" OFF)
sosage_option(SOSAGE_CFG_USE_SDL_TIME "Use SDL clock instead of STL clock" OFF)
sosage_option(SOSAGE_CFG_ASSERTIONS_AS_EXCEPTIONS "Assertions treated as exceptions instead of triggering exit()" ON)
sosage_option(SOSAGE_COMPILE_TESTS "Compile test executables" OFF)

## Check all dependencies

set(SOSAGE_DEPENDENCIES_OKAY true)

find_package(YAML REQUIRED)
if (NOT YAML_FOUND)
  message(STATUS "libyaml is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

find_package(SDL2 REQUIRED)
if (NOT SDL2_FOUND)
  message(STATUS "SDL2 is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

find_package(SDL2_mixer QUIET)
if (NOT SDL2_MIXER_FOUND)
  message(STATUS "SDL2 Mixer is optional and not found (no sound will be available)")
endif()

find_package(SDL2_image REQUIRED)
if (NOT SDL2_IMAGE_FOUND)
  message(STATUS "SDL2 Image is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

find_package(SDL2_ttf REQUIRED)
if (NOT SDL2_ttf_FOUND)
  message(STATUS "SDL2 TTF is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

## Abort if at least one dependency is missing

if (NOT SOSAGE_DEPENDENCIES_OKAY)
  message(STATUS "Some dependencies are missing (see below), project won't be compiled")
  return()
endif()

## Set up data folder

set(SOSAGE_DATA_FOLDER "" CACHE PATH "Where the game data is")

if (SOSAGE_DATA_FOLDER STREQUAL "")
  message(STATUS "No data folder provided, nothing will be compiled")
  return()
endif()

include(${SOSAGE_DATA_FOLDER}/config.cmake)
include(Sosage_version)
include(Sosage_utils)

## Fetch sources and create executable

file(GLOB_RECURSE SOSAGE_SRC "src/Sosage/*.cpp")

if(MINGW)
 set(SOSAGE_SRC ${SOSAGE_SRC} "${SOSAGE_DATA_FOLDER}/resources/sosage.rc")
 set(CMAKE_RC_COMPILER_INIT windres)
 enable_language(RC)
 set(CMAKE_RC_COMPILE_OBJECT
 "<CMAKE_RC_COMPILER> <FLAGS> -O coff <DEFINES> -i <SOURCE> -o <OBJECT>")
endif(MINGW)

add_executable(${SOSAGE_EXE_NAME} ${SOSAGE_SRC})
target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC sosage)

## Platform-specific instructions
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_INSTALL_DATA_FOLDER="../share/${SOSAGE_EXE_NAME}/")
  install(TARGETS ${SOSAGE_EXE_NAME} DESTINATION bin)
  install(DIRECTORY ${SOSAGE_DATA_FOLDER}/data/ DESTINATION share/${SOSAGE_EXE_NAME})
  install(FILES ${SOSAGE_DATA_FOLDER}/resources/icon.svg DESTINATION share/icons/hicolor/scalable/apps/ RENAME ${SOSAGE_EXE_NAME}.svg)
  install(FILES ${SOSAGE_DATA_FOLDER}/resources/${SOSAGE_EXE_NAME}.desktop DESTINATION share/applications/)
  set(CPACK_GENERATOR "DEB;RPM")
elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
  target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_INSTALL_DATA_FOLDER="data/")
  install(TARGETS ${SOSAGE_EXE_NAME} DESTINATION ".")
  install(DIRECTORY ${SOSAGE_DATA_FOLDER}/data/ DESTINATION "./data")
  install(FILES ${SOSAGE_SYSTEM_DLLS} DESTINATION ".")
  install(FILES ${SOSAGE_LOCAL_DLLS} DESTINATION ".")
  set(CPACK_GENERATOR "NSIS")
elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)

endif()

include(CPack)

if(SOSAGE_COMPILE_TESTS)

  list(REMOVE_ITEM SOSAGE_SRC ${CMAKE_SOURCE_DIR}/src/Sosage/Sosage.cpp)

  enable_testing()

  set (SOSAGE_TEST_TARGETS "unit_Component_Ground_map")
  set (SOSAGE_TEST_TARGETS ${SOSAGE_TEST_TARGETS} "check_data_validity")

  foreach(target ${SOSAGE_TEST_TARGETS})
    add_executable("test_${target}" "test/${target}.cpp" ${SOSAGE_SRC})
    target_link_libraries("test_${target}" PUBLIC sosage)
    sosage_add_test("test_${target}")
  endforeach()

  file(GLOB_RECURSE GROUND_MAPS "${SOSAGE_DATA_FOLDER}/data/images/backgrounds/*_map.png")
  list(JOIN GROUND_MAPS "\;" GROUND_MAPS_STR)
  target_compile_definitions("test_unit_Component_Ground_map" PUBLIC "-DSOSAGE_GROUND_MAPS=\"${GROUND_MAPS_STR}\"")

  file(GLOB_RECURSE DATA_FILES "${SOSAGE_DATA_FOLDER}/data/data/*/*.yaml")
  list(JOIN DATA_FILES "\;" DATA_FILES_STR)
  target_compile_definitions("test_check_data_validity" PUBLIC "-DSOSAGE_DATA_FILES=\"${DATA_FILES_STR}\"")

  file(GLOB_RECURSE SOSAGE_HEADERS "include/Sosage/*.h")
  foreach(header ${SOSAGE_HEADERS})
    string(REPLACE "/" "_" target ${header})
    string(REPLACE "." "_" target ${target})
    add_executable("check_header_${target}" "test/check_header.cpp")
    target_compile_definitions("check_header_${target}" PUBLIC "-DSOSAGE_TESTED_HEADER=\"${header}\"")
    target_link_libraries("check_header_${target}" PUBLIC sosage)
    sosage_add_test("check_header_${target}")
  endforeach()

endif()
