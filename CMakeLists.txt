cmake_minimum_required (VERSION 3.2)
project (superflu_pnc)

## Use C++14 and load local CMake modules

set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

## Optional builds

string(TOUPPER ${CMAKE_SYSTEM_NAME} HOST_OS)
option(SUPERFLU_PNC_BUILD_FOR_${HOST_OS} "Build project for host operating system (${CMAKE_SYSTEM_NAME})" ON)
option(SUPERFLU_PNC_BUILD_FOR_ANDROID "Build project for Android devices" OFF)
option(SUPERFLU_PNC_BUILD_FOR_BROWSER "Build project for browser integration" OFF)

if(SUPERFLU_PNC_BUILD_FOR_${HOST_OS})

  ## Check all dependencies

  set(SUPERFLU_PNC_DEPENDENCIES_OKAY true)

  find_package(LibXml2 REQUIRED)
  if (NOT LIBXML2_FOUND)
    message(STATUS "LibXml2 is required and not found")
    set(SUPERFLU_PNC_DEPENDENCIES_OKAY false)
  endif()
  
  find_package(SDL2 REQUIRED)
  if (NOT SDL2_FOUND)
    message(STATUS "SDL2 is required and not found")
    set(SUPERFLU_PNC_DEPENDENCIES_OKAY false)
  endif()  

  find_package(SDL2_mixer REQUIRED)
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

  file(GLOB_RECURSE SUPERFLU_PNC_SRC "src/*.cpp")
  add_executable(superflu_pnc ${SUPERFLU_PNC_SRC})

  ## Set up data folder

  target_compile_options(superflu_pnc PUBLIC -DSUPERFLU_DATA_FOLDER="${CMAKE_CURRENT_LIST_DIR}/superflu/")

  ## Compilation dependencies

  target_include_directories (superflu_pnc PUBLIC
    "include"
    ${LIBXML2_INCLUDE_DIRS}
    ${SDL2_INCLUDE_DIR}
    ${SDL2_MIXER_INCLUDE_DIR}
    ${SDL2_IMAGE_INCLUDE_DIR}
    ${SDL2_TTF_INCLUDE_DIR})

  ## Link dependencies

  target_link_libraries(superflu_pnc PUBLIC
    ${LIBXML2_LIBRARIES}
    ${SDL2_LIBRARY}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES})

  ## Optional sound
  if (SDL2_MIXER_FOUND)
    target_include_directories (superflu_pnc PUBLIC ${SDL2_MIXER_INCLUDE_DIR})
    target_link_libraries(superflu_pnc PUBLIC ${SDL2_MIXER_LIBRARIES})
    target_compile_definitions(superflu_pnc PUBLIC "-DSOSAGE_LINKED_WITH_SDL_MIXER")
  endif()

endif()

if(SUPERFLU_PNC_BUILD_FOR_ANDROID)

endif()

if(SUPERFLU_PNC_BUILD_FOR_BROWSER)

endif()
