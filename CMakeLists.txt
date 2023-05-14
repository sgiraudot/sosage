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

set(SOSAGE_DATA_FOLDER "" CACHE PATH "Where the game data is")

if (SOSAGE_DATA_FOLDER STREQUAL "")
  message(STATUS "No data folder provided, nothing will be compiled")
  return()
endif()

include(${SOSAGE_DATA_FOLDER}/config.cmake)

# Native Android compilation (on Termux for example)
# To compile APK, use Gradlew instead
if (CMAKE_SYSTEM_NAME STREQUAL Android)
  add_definitions(-DSOSAGE_NATIVE_ANDROID)
endif()

if (CMAKE_SYSTEM_NAME STREQUAL Emscripten)
  include(Sosage_version)
  file(GLOB_RECURSE SOSAGE_SRC "src/Sosage/*.cpp")
  set(YAML_SOURCE_PATH "" CACHE PATH "Source of the YAML library")
  set(LZ4_SOURCE_PATH "" CACHE PATH "Source of the LZ4 library")
  file(GLOB_RECURSE YAML_SRC "${YAML_SOURCE_PATH}/src/*.c")
  file(GLOB_RECURSE LZ4_SRC "${LZ4_SOURCE_PATH}/lib/*.c")

  if (EXISTS ${SOSAGE_DATA_FOLDER}/data/general.data)
    file(GLOB_RECURSE SOSAGE_COMPRESSED_DATA "${SOSAGE_DATA_FOLDER}/data/*.data")
    file(COPY ${SOSAGE_COMPRESSED_DATA} DESTINATION "${CMAKE_BINARY_DIR}/data/")
  else()
    file(COPY "${SOSAGE_DATA_FOLDER}/data/" DESTINATION "${CMAKE_BINARY_DIR}/data/")
  endif()

  add_executable(${SOSAGE_EXE_NAME} ${SOSAGE_SRC} ${YAML_SRC} ${LZ4_SRC})
  target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_DATA_FOLDER="data/")
  target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_INSTALL_DATA_FOLDER="data/")
  target_compile_definitions(${SOSAGE_EXE_NAME} PUBLIC "-DSOSAGE_LINKED_WITH_SDL_MIXER")
  target_include_directories (${SOSAGE_EXE_NAME} PUBLIC
    "include"
    "${YAML_SOURCE_PATH}/include"
    "${YAML_SOURCE_PATH}/src"
    "${LZ4_SOURCE_PATH}/lib")

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
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --preload-file data")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s DISABLE_EXCEPTION_CATCHING=0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s ALLOW_MEMORY_GROWTH=1")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_IMAGE=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s SDL2_IMAGE_FORMATS='[\"png\"]'")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_MIXER=2")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_SDL_TTF=2")
  target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-s WASM=1")
  target_link_libraries(${SOSAGE_EXE_NAME} PUBLIC "-o ${SOSAGE_EXE_NAME}.html")

  return()
endif()

sosage_option(SOSAGE_CFG_DISPLAY_DEBUG_INFO "Force displaying debug info even in Release mode" OFF)
sosage_option(SOSAGE_CFG_PROFILE "Profile and display timing info (requires debug info ON)" OFF)
sosage_option(SOSAGE_CFG_USE_SDL_TIME "Use SDL clock instead of STL clock" OFF)
sosage_option(SOSAGE_CFG_USE_SDL_MIXER_EXT "Use more advanced fork of SDL_Mixer" OFF)
sosage_option(SOSAGE_CFG_GUILESS "Do not instanciate window/renderer, for testing purposes" OFF)
sosage_option(SOSAGE_COMPILE_SCAP "Compile Sosage Compressed Data Packager" OFF)
sosage_option(SOSAGE_CONFIG_ANDROID "Configure Sosage for Android" OFF)

set(STEAMSDK_ROOT "" CACHE PATH "Root of Steam SDK (optional)")
set(SOSAGE_STEAM_APP_ID "" CACHE STRING "App ID of the game on Steam (optional)")

set(SOSAGE_BUILD_TYPE "Release" CACHE STRING "Is sosage built for Release/Beta/Demo/Dev (this is different from CMAKE_BUILD_TYPE)")
if (SOSAGE_BUILD_TYPE STREQUAL "Release")
  add_definitions(-DSOSAGE_RELEASE)
elseif (SOSAGE_BUILD_TYPE STREQUAL "Beta")
  add_definitions(-DSOSAGE_BETA)
elseif (SOSAGE_BUILD_TYPE STREQUAL "Demo")
  add_definitions(-DSOSAGE_DEMO)
elseif (SOSAGE_BUILD_TYPE STREQUAL "Dev")
  add_definitions(-DSOSAGE_DEV)
else()
  message(STATUS "Unknown SOSAGE_BUILD_TYPE ${SOSAGE_BUILD_TYPE}, possible values are Release, Beta, Demo or Dev")
  return()
endif()


## Check all dependencies

set(SOSAGE_DEPENDENCIES_OKAY true)
set(SOSAGE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/include")
set(SOSAGE_LINK_LIBRARIES "")
set(SOSAGE_COMPILE_DEFINITIONS "")
list(APPEND SOSAGE_COMPILE_DEFINITIONS "SOSAGE_DATA_FOLDER=\"${SOSAGE_DATA_FOLDER}/data/\"")
list(APPEND SOSAGE_COMPILE_DEFINITIONS "SOSAGE_PREF_PATH=\"${SOSAGE_EDITOR_PATH}\"")
list(APPEND SOSAGE_COMPILE_DEFINITIONS "SOSAGE_PREF_SUBPATH=\"${SOSAGE_EXE_NAME}\"")

find_package(YAML REQUIRED)
if (YAML_FOUND)
  list(APPEND SOSAGE_INCLUDE_DIRECTORIES ${YAML_INCLUDE_DIR})
  list(APPEND SOSAGE_LINK_LIBRARIES ${YAML_LIBRARIES})
else()
  message(STATUS "libyaml is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

find_package(SDL2 REQUIRED)
if (SDL2_FOUND)
  list(APPEND SOSAGE_INCLUDE_DIRECTORIES ${SDL2_INCLUDE_DIR})
  list(APPEND SOSAGE_LINK_LIBRARIES ${SDL2_LIBRARY})
else()
  message(STATUS "SDL2 is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

if (SOSAGE_CFG_USE_SDL_MIXER_EXT)
  find_package(SDL2_mixer_ext QUIET)
  if (SDL2_MIXER_EXT_FOUND)
    list(APPEND SOSAGE_INCLUDE_DIRECTORIES ${SDL2_MIXER_EXT_INCLUDE_DIR})
    list(APPEND SOSAGE_LINK_LIBRARIES ${SDL2_MIXER_EXT_LIBRARIES})
    list(APPEND SOSAGE_COMPILE_DEFINITIONS "SOSAGE_LINKED_WITH_SDL_MIXER_EXT")
  else()
    message(STATUS "SDL2 Mixer Ext is optional and not found (no sound will be available)")
  endif()
else()
  find_package(SDL2_mixer QUIET)
  if (SDL2_MIXER_FOUND)
    list(APPEND SOSAGE_INCLUDE_DIRECTORIES ${SDL2_MIXER_INCLUDE_DIR})
    list(APPEND SOSAGE_LINK_LIBRARIES ${SDL2_MIXER_LIBRARIES})
    list(APPEND SOSAGE_COMPILE_DEFINITIONS "SOSAGE_LINKED_WITH_SDL_MIXER")
  else()
    message(STATUS "SDL2 Mixer is optional and not found (no sound will be available)")
  endif()
endif()

find_package(SDL2_image REQUIRED)
if (SDL2_IMAGE_FOUND)
  list(APPEND SOSAGE_INCLUDE_DIRECTORIES ${SDL2_IMAGE_INCLUDE_DIR})
  list(APPEND SOSAGE_LINK_LIBRARIES ${SDL2_IMAGE_LIBRARIES})
else()
  message(STATUS "SDL2 Image is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

find_package(SDL2_ttf REQUIRED)
if (SDL2_ttf_FOUND)
  list(APPEND SOSAGE_INCLUDE_DIRECTORIES ${SDL2_TTF_INCLUDE_DIR})
  list(APPEND SOSAGE_LINK_LIBRARIES ${SDL2_TTF_LIBRARIES})
else()
  message(STATUS "SDL2 TTF is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

find_package(LZ4 REQUIRED)
if (LZ4_FOUND)
  list(APPEND SOSAGE_INCLUDE_DIRECTORIES ${LZ4_INCLUDE_DIR})
  list(APPEND SOSAGE_LINK_LIBRARIES ${LZ4_LIBRARY})
else()
  message(STATUS "LZ4 is required and not found")
  set(SOSAGE_DEPENDENCIES_OKAY false)
endif()

if(NOT STEAMSDK_ROOT STREQUAL "" AND NOT SOSAGE_STEAM_APP_ID STREQUAL "")
  list(APPEND SOSAGE_COMPILE_DEFINITIONS "SOSAGE_LINKED_WITH_STEAMSDK")
  list(APPEND SOSAGE_COMPILE_DEFINITIONS "SOSAGE_STEAM_APP_ID=${SOSAGE_STEAM_APP_ID}")
  list(APPEND SOSAGE_INCLUDE_DIRECTORIES "${STEAMSDK_ROOT}/public/")
  list(APPEND SOSAGE_LINK_LIBRARIES "${STEAMSDK_ROOT}/redistributable_bin/linux64/libsteam_api.so")
endif()

## Abort if at least one dependency is missing

if (NOT SOSAGE_DEPENDENCIES_OKAY)
  message(STATUS "Some dependencies are missing (see below), project won't be compiled")
  return()
endif()

## Set up data folder

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
target_include_directories(${SOSAGE_EXE_NAME} PUBLIC ${SOSAGE_INCLUDE_DIRECTORIES})
target_link_libraries(${SOSAGE_EXE_NAME} ${SOSAGE_LINK_LIBRARIES})
target_compile_definitions(${SOSAGE_EXE_NAME} PUBLIC ${SOSAGE_COMPILE_DEFINITIONS})

## Platform-specific instructions
if (CMAKE_SYSTEM_NAME STREQUAL Linux)
  target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_INSTALL_DATA_FOLDER="../share/${SOSAGE_EXE_NAME}/")
  install(TARGETS ${SOSAGE_EXE_NAME} DESTINATION bin)
  if (EXISTS ${SOSAGE_DATA_FOLDER}/data/general.data)
    file(GLOB_RECURSE SOSAGE_COMPRESSED_DATA "${SOSAGE_DATA_FOLDER}/data/*.data")
    install(FILES ${SOSAGE_COMPRESSED_DATA} DESTINATION share/${SOSAGE_EXE_NAME})
  else()
    install(DIRECTORY ${SOSAGE_DATA_FOLDER}/data/ DESTINATION share/${SOSAGE_EXE_NAME})
  endif()
  install(FILES ${SOSAGE_DATA_FOLDER}/resources/icon.svg DESTINATION share/icons/hicolor/scalable/apps/ RENAME ${SOSAGE_EXE_NAME}.svg)
  install(FILES ${SOSAGE_DATA_FOLDER}/resources/${SOSAGE_EXE_NAME}.desktop DESTINATION share/applications/)
  set(CPACK_GENERATOR "DEB;RPM")

elseif (CMAKE_SYSTEM_NAME STREQUAL Windows)
  target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_INSTALL_DATA_FOLDER="data/")
  install(TARGETS ${SOSAGE_EXE_NAME} DESTINATION ".")
  if (EXISTS ${SOSAGE_DATA_FOLDER}/data/general.data)
    file(GLOB_RECURSE SOSAGE_COMPRESSED_DATA "${SOSAGE_DATA_FOLDER}/data/*.data")
    install(FILES ${SOSAGE_COMPRESSED_DATA} DESTINATION "./data")
  else()
    install(DIRECTORY ${SOSAGE_DATA_FOLDER}/data/ DESTINATION "./data")
  endif()
  install(FILES ${SOSAGE_SYSTEM_DLLS} DESTINATION ".")
  install(FILES ${SOSAGE_LOCAL_DLLS} DESTINATION ".")
  set(CPACK_GENERATOR "NSIS")

elseif (CMAKE_SYSTEM_NAME STREQUAL Darwin)
  target_compile_options(${SOSAGE_EXE_NAME} PUBLIC -DSOSAGE_INSTALL_DATA_FOLDER="data/")
  install(TARGETS ${SOSAGE_EXE_NAME} DESTINATION "${SOSAGE_NAME}.app/Contents/MacOS")
  if (EXISTS ${SOSAGE_DATA_FOLDER}/data/general.data)
    file(GLOB_RECURSE SOSAGE_COMPRESSED_DATA "${SOSAGE_DATA_FOLDER}/data/*.data")
    install(FILES ${SOSAGE_COMPRESSED_DATA} DESTINATION "${SOSAGE_NAME}.app/Contents/Resources/data")
  else()
    install(DIRECTORY ${SOSAGE_DATA_FOLDER}/data/ DESTINATION "${SOSAGE_NAME}.app/Contents/Resources/data")
  endif()
  foreach(lib ${CMAKE_REQUIRED_MAC_LIBS})
    install(FILES ${CMAKE_FIND_ROOT_PATH}/lib/${lib} DESTINATION "${SOSAGE_NAME}.app/Contents/libs/")
  endforeach()
  install(FILES "${SOSAGE_DATA_FOLDER}/resources/Info.plist" DESTINATION "${SOSAGE_NAME}.app/Contents/")
  install(FILES "${SOSAGE_DATA_FOLDER}/resources/icon.icns" DESTINATION "${SOSAGE_NAME}.app/Contents/Resources")
  set(CPACK_GENERATOR "ZIP")

endif()

if (SOSAGE_CONFIG_ANDROID)

  if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_ANDROID_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
  elseif (CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    set(CMAKE_ANDROID_FLAGS ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})
  elseif (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_ANDROID_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
  endif()

  if(SOSAGE_CFG_TEST_INPUT)
    set(CMAKE_ANDROID_FLAGS "${CMAKE_ANDROID_FLAGS} -DSOSAGE_CFG_TEST_INPUT")
  endif()
  if(SOSAGE_CFG_DISPLAY_DEBUG_INFO)
    set(CMAKE_ANDROID_FLAGS "${CMAKE_ANDROID_FLAGS} -DSOSAGE_CFG_DISPLAY_DEBUG_INFO")
  endif()

  set(SDL2_SOURCE_PATH "" CACHE PATH "Source of the SDL library")
  set(SDL2_IMAGE_SOURCE_PATH "" CACHE PATH "Source of the SDL Image library")
  set(SDL2_MIXER_SOURCE_PATH "" CACHE PATH "Source of the SDL Mixer (or SDL Mixer EXT) library")
  set(SDL2_TTF_SOURCE_PATH "" CACHE PATH "Source of the SDL TTF library")
  set(YAML_SOURCE_PATH "" CACHE PATH "Source of the YAML library")
  set(LZ4_SOURCE_PATH "" CACHE PATH "Source of the LZ4 library")
  set(SOSAGE_KEYSTORE_ALIAS "" CACHE STRING "Alias of the keystore file for Android signing")
  set(SOSAGE_KEYSTORE_PASSWORD "" CACHE STRING "Password of the keystore file for Android signing")
  set(HACK_VARIANT_NAME "\$\{variant.name.capitalize()\}")

  if (SOSAGE_CFG_USE_SDL_MIXER_EXT)
    set(SDL2_MIXER_ANDROID_MK "${CMAKE_BINARY_DIR}/android/app/jni/SDL_mixer_ext/Android.mk")
    set(CMAKE_ANDROID_FLAGS "${CMAKE_ANDROID_FLAGS} -DSOSAGE_LINKED_WITH_SDL_MIXER_EXT")
  else()
    set(SDL2_MIXER_ANDROID_MK "${SDL2_MIXER_SOURCE_PATH}/Android.mk")
    set(CMAKE_ANDROID_FLAGS "${CMAKE_ANDROID_FLAGS} -DSOSAGE_LINKED_WITH_SDL_MIXER")
  endif()


  file(COPY "${CMAKE_SOURCE_DIR}/platform/android/" DESTINATION "${CMAKE_BINARY_DIR}/android/")

  if (EXISTS ${SOSAGE_DATA_FOLDER}/data/general.data)
    file(GLOB_RECURSE SOSAGE_COMPRESSED_DATA "${SOSAGE_DATA_FOLDER}/data/*.data")
    file(COPY ${SOSAGE_COMPRESSED_DATA} DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/assets/data/")
  else()
    file(COPY "${SOSAGE_DATA_FOLDER}/data/" DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/assets/data/")
  endif()

  file(COPY "${SOSAGE_DATA_FOLDER}/resources/icon.png"
    DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxxhdpi/")
  file(COPY "${SOSAGE_DATA_FOLDER}/resources/icon144x144.png"
    DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxhdpi/")
  file(COPY "${SOSAGE_DATA_FOLDER}/resources/icon96x96.png"
    DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xhdpi/")
  file(COPY "${SOSAGE_DATA_FOLDER}/resources/icon72x72.png"
    DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-hdpi/")
  file(COPY "${SOSAGE_DATA_FOLDER}/resources/icon48x48.png"
    DESTINATION "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-mdpi/")

  file(RENAME "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxxhdpi/icon.png"
    "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxxhdpi/ic_launcher.png")
  file(RENAME "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxhdpi/icon144x144.png"
    "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xxhdpi/ic_launcher.png")
  file(RENAME "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xhdpi/icon96x96.png"
    "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-xhdpi/ic_launcher.png")
  file(RENAME "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-hdpi/icon72x72.png"
    "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-hdpi/ic_launcher.png")
  file(RENAME "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-mdpi//icon48x48.png"
    "${CMAKE_BINARY_DIR}/android/app/src/main/res/mipmap-mdpi/ic_launcher.png")

  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_build.gradle.in"
    "${CMAKE_BINARY_DIR}/android/app/build.gradle")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_jni_Android.mk.in"
    "${CMAKE_BINARY_DIR}/android/app/jni/Android.mk")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_jni_LZ4_Android.mk.in"
    "${CMAKE_BINARY_DIR}/android/app/jni/LZ4/Android.mk")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_jni_SDL_mixer_ext_Android.mk.in"
    "${CMAKE_BINARY_DIR}/android/app/jni/SDL_mixer_ext/Android.mk")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_jni_src_Android.mk.in"
    "${CMAKE_BINARY_DIR}/android/app/jni/src/Android.mk")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_jni_yaml_Android.mk.in"
    "${CMAKE_BINARY_DIR}/android/app/jni/yaml/Android.mk")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_src_main_AndroidManifest.xml.in"
    "${CMAKE_BINARY_DIR}/android/app/src/main/AndroidManifest.xml")
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_src_main_java_net_main.java.in"
    "${CMAKE_BINARY_DIR}/android/app/src/main/java/${SOSAGE_EDITOR_EXT}/${SOSAGE_EDITOR_PATH}/${SOSAGE_ACTIVITY}/${SOSAGE_ACTIVITY}.java")

  set(SOSAGE_LOCALE_NAME ${SOSAGE_NAME})
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_src_main_res_values_strings.xml.in"
    "${CMAKE_BINARY_DIR}/android/app/src/main/res/values/strings.xml")
  set(SOSAGE_LOCALE_NAME ${SOSAGE_NAME_FR})
  configure_file("${CMAKE_SOURCE_DIR}/cmake/android/app_src_main_res_values_strings.xml.in"
    "${CMAKE_BINARY_DIR}/android/app/src/main/res/values-fr/strings.xml")
endif()

include(CPack)

if(SOSAGE_COMPILE_SCAP)
  set(SCAP_SRC "src/Sosage/Component/Base.cpp" "src/Sosage/Component/Ground_map.cpp"
    "src/Sosage/Third_party/LZ4.cpp" "src/Sosage/Third_party/SDL.cpp"  "src/Sosage/Third_party/SDL_file.cpp"
    "src/Sosage/Utils/asset_packager.cpp" "src/Sosage/Utils/Asset_manager.cpp" "src/Sosage/Utils/Bitmap_2.cpp"
     "src/Sosage/Utils/binary_io.cpp" "src/Sosage/Utils/color.cpp"
    "src/Sosage/Utils/conversions.cpp" "src/Sosage/Utils/error.cpp" "src/Sosage/Utils/geometry.cpp"
    "src/Sosage/Utils/image_split.cpp" "src/Sosage/Utils/profiling.cpp")
  add_executable("SCAP" "src/Sosage/SCAP.cpp" ${SCAP_SRC})
  target_include_directories(SCAP PUBLIC ${SOSAGE_INCLUDE_DIRECTORIES})
  target_link_libraries(SCAP ${SOSAGE_LINK_LIBRARIES} "tbb")
  target_compile_definitions(SCAP PUBLIC
    ${SOSAGE_COMPILE_DEFINITIONS}
    "SOSAGE_SCAP")
endif()
