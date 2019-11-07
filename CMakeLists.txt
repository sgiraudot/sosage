cmake_minimum_required (VERSION 3.2)
project (sosage)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-pthread")
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

set(project_linked_libraries)

find_package(LibXml2 REQUIRED)
if (LIBXML2_FOUND)
  include_directories(${LIBXML2_INCLUDE_DIRS})
   set(project_linked_libraries ${project_linked_libraries}
     ${LIBXML2_LIBRARIES})
else()
  message(STATUS "LibXml2 is required and not found, nothing will be compiled.")
  return()
endif()
  
find_package(SDL REQUIRED)
find_package(SDL_mixer REQUIRED)
find_package(SDL_image REQUIRED)
find_package(SDL_gfx REQUIRED)
find_package(SDL_ttf REQUIRED)
if(SDL_FOUND AND SDL_MIXER_FOUND AND SDL_image_FOUND AND SDL_GFX_FOUND AND SDL_ttf_FOUND)
  set(project_linked_libraries ${project_linked_libraries}
    ${SDL_LIBRARY}
    ${SDL_MIXER_LIBRARIES}
    ${SDL_IMAGE_LIBRARIES}
    ${SDL_GFX_LIBRARIES}
    ${SDL_TTF_LIBRARIES})
  include_directories(${SDL_INCLUDE_DIR})
  include_directories(${SDL_MIXER_INCLUDE_DIR})
  include_directories(${SDL_IMAGE_INCLUDE_DIR})
  include_directories(${SDL_GFX_INCLUDE_DIR})
  include_directories(${SDL_TTF_INCLUDE_DIR})
else()
  message(STATUS "SDL is required and not found, only models will be compiled. Missing components:")
  if (NOT SDL_FOUND)
    message(STATUS " * SDL")
  endif()
  if (NOT SDL_MIXER_FOUND)
    message(STATUS " * SDL Mixer")
  endif()
  if (NOT SDL_IMAGE_FOUND)
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

include_directories ("include")
file(GLOB_RECURSE sosage_SRC "src/*.cpp")

add_library(sosage SHARED ${sosage_SRC})
target_link_libraries(sosage PUBLIC ${project_linked_libraries})

add_executable(sosage_test "test/test.cpp")
target_link_libraries(sosage_test PUBLIC sosage)

