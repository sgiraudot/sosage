cmake_minimum_required (VERSION 3.2)
project (sosage)

set(CMAKE_CXX_STANDARD 14)
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
  
find_package(SDL2 REQUIRED)
find_package(SDL2_mixer REQUIRED)
find_package(SDL2_image REQUIRED)
find_package(SDL2_ttf REQUIRED)
if(SDL2_FOUND AND SDL2_MIXER_FOUND AND SDL2_image_FOUND AND SDL2_ttf_FOUND)
  set(project_linked_libraries ${project_linked_libraries}
    ${SDL2_LIBRARY}
    ${SDL2_MIXER_LIBRARIES}
    ${SDL2_IMAGE_LIBRARIES}
    ${SDL2_TTF_LIBRARIES})
  include_directories(${SDL2_INCLUDE_DIR})
  include_directories(${SDL2_MIXER_INCLUDE_DIR})
  include_directories(${SDL2_IMAGE_INCLUDE_DIR})
  include_directories(${SDL2_TTF_INCLUDE_DIR})
else()
  message(STATUS "SDL2 is required and not found, only models will be compiled. Missing components:")
  if (NOT SDL2_FOUND)
    message(STATUS " * SDL2")
  endif()
  if (NOT SDL2_MIXER_FOUND)
    message(STATUS " * SDL2 Mixer")
  endif()
  if (NOT SDL2_IMAGE_FOUND)
    message(STATUS " * SDL2 Image")
  endif()
  if (NOT SDL2_TTF_FOUND)
    message(STATUS " * SDL2 TTF")
  endif()
  
  return()
endif()

include_directories ("include")
file(GLOB_RECURSE sosage_SRC "src/*.cpp")

add_library(sosage SHARED ${sosage_SRC})
target_link_libraries(sosage PUBLIC ${project_linked_libraries})

add_executable(superflu "test/superflu.cpp")
target_link_libraries(superflu PUBLIC sosage)
target_compile_options(superflu PUBLIC -DSUPERFLU_DATA_FOLDER="${CMAKE_CURRENT_LIST_DIR}/superflu/")
