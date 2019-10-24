cmake_minimum_required (VERSION 3.2)
project (sosage)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(project_linked_libraries)
set(project_compilation_definitions)

find_package(Lua51 REQUIRED)
if (Lua51_FOUND)
  include_directories(${LUA_INCLUDE_DIR})
  set(project_linked_libraries ${project_linked_libraries}
    ${LUA_LIBRARIES})
else()
  message(STATUS "LUA 5.1 is required and not found, nothing will be compiled.")
  return()
endif()

include_directories ("include")
file(GLOB_RECURSE sosage_SRC "src/*.cpp")

add_library(sosage_model SHARED ${sosage_SRC})
target_link_libraries(sosage_model PUBLIC ${project_linked_libraries})
target_compile_definitions(sosage_model PUBLIC ${project_compilation_definitions})

add_executable(sosage_model_test "test/test.cpp")
target_link_libraries(sosage_model_test PUBLIC sosage_model)
target_link_libraries(sosage_model_test PUBLIC ${project_linked_libraries})

find_package(SDL QUIET)
find_package(SDL_image QUIET)
find_package(SDL_gfx QUIET)
find_package(SDL_ttf QUIET)
if(SDL_FOUND AND SDL_image_FOUND AND SDL_GFX_FOUND AND SDL_ttf_FOUND)
  set(project_linked_libraries ${project_linked_libraries}
    ${SDL_LIBRARY}
    ${SDL_IMAGE_LIBRARIES}
    ${SDL_GFX_LIBRARIES}
    ${SDL_TTF_LIBRARIES})
  set(project_compilation_definitions ${project_compilation_definitions}
    "-DSOSAGE_LINKED_WITH_SDL")
  include_directories(${SDL_INCLUDE_DIR})
  include_directories(${SDL_IMAGE_INCLUDE_DIR})
  include_directories(${SDL_GFX_INCLUDE_DIR})
  include_directories(${SDL_TTF_INCLUDE_DIR})
else()
  message(STATUS "SDL is required and not found, only models will be compiled. Missing components:")
  if (NOT SDL_FOUND)
    message(STATUS " * SDL")
  endif()
  if (NOT SDL_image_FOUND)
    message(STATUS " * SDL Image")
  endif()
  if (NOT SDL_GFX_FOUND)
    message(STATUS " * SDL GFX")
  endif()
  if (NOT SDL_TTF_FOUND)
    message(STATUS " * SDL TTF")
  endif()
  
  return()
endif()

find_package(SDL_mixer QUIET)
if(SDL_mixer_FOUND)
  set(project_linked_libraries ${project_linked_libraries}
    ${SDL_MIXER_LIBRARIES})
  set(project_compilation_definitions ${project_compilation_definitions}
    "-DSOSAGE_LINKED_WITH_SDL_MIXER")
  include_directories(${SDL_MIXER_INCLUDE_DIR})
else()
  message(STATUS "SDL_mixer is required and not found, no sound will be available.")
endif()

add_library(sosage SHARED ${sosage_SRC})
target_link_libraries(sosage PUBLIC ${project_linked_libraries})
target_compile_definitions(sosage PUBLIC ${project_compilation_definitions})

add_executable(sosage_test "test/test.cpp")
target_link_libraries(sosage_test PUBLIC sosage)
target_link_libraries(sosage_test PUBLIC ${project_linked_libraries})

