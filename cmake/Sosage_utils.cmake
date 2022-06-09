add_library(sosage INTERFACE IMPORTED)
set_target_properties(sosage PROPERTIES
  INTERFACE_COMPILE_DEFINITIONS "SOSAGE_DATA_FOLDER=\"${SOSAGE_DATA_FOLDER}/data/\";SOSAGE_LINKED_WITH_SDL_MIXER;SOSAGE_PREF_PATH=\"${SOSAGE_EDITOR_PATH}\";SOSAGE_PREF_SUBPATH=\"${SOSAGE_EXE_NAME}\""
  INTERFACE_INCLUDE_DIRECTORIES   "${CMAKE_SOURCE_DIR}/include;${YAML_INCLUDE_DIR};${SDL2_INCLUDE_DIR};${SDL2_IMAGE_INCLUDE_DIR};${SDL2_TTF_INCLUDE_DIR};${SDL2_MIXER_INCLUDE_DIR};${LZ4_INCLUDE_DIR}"
  INTERFACE_LINK_LIBRARIES "${YAML_LIBRARIES};${SDL2_LIBRARY};${SDL2_IMAGE_LIBRARIES};${SDL2_TTF_LIBRARIES};${SDL2_MIXER_LIBRARIES};${LZ4_LIBRARY}")

function(sosage_add_test target)
  add_test(NAME "compiling_${target}" COMMAND "${CMAKE_COMMAND}" --build "${CMAKE_BINARY_DIR}" --target "${target}")
  add_test(NAME "running_${target}" COMMAND "${target}")
endfunction()

set(CPACK_PACKAGE_NAME ${SOSAGE_EXE_NAME})
set(CPACK_PACKAGE_VENDOR ${SOSAGE_EDITOR})
set(CPACK_PACKAGE_CONTACT ${SOSAGE_AUTHOR})
set(CPACK_PACKAGE_VERSION_MAJOR ${SOSAGE_VERSION_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${SOSAGE_VERSION_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${SOSAGE_VERSION_PATCH})
set(CPACK_PACKAGE_DESCRIPTION ${SOSAGE_DESCRIPTION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY ${SOSAGE_DESCRIPTION})
set(CPACK_PACKAGE_HOMEPAGE_URL ${SOSAGE_URL})
set(CPACK_PACKAGE_ICON ${SOSAGE_ICON})
set(CPACK_PACKAGE_EXECUTABLES "${SOSAGE_EXE_NAME};${SOSAGE_NAME}")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/LICENSE.md")

set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libsdl2-2.0-0 (>=2.0.10), libsdl2-image-2.0-0, libsdl2-mixer-2.0-0, libsdl2-ttf-2.0-0, libyaml-0-2, liblz4-1")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE ${SOSAGE_URL})

set(CPACK_RPM_PACKAGE_GROUP "Games")
set(CPACK_RPM_PACKAGE_REQUIRES "SDL2 >= 2.0.10, SDL2_image, SDL2_mixer, SDL2_ttf, libyaml, lz4-libs")
set(CPACK_RPM_PACKAGE_URL ${SOSAGE_URL})
set(CPACK_RPM_EXCLUDE_FROM_AUTO_FILELIST_ADDITION /usr/share/icons /usr/share/icons/hicolor /usr/share/icons/hicolor/scalable /usr/share/icons/hicolor/scalable/apps /usr/share/applications)

set(CPACK_NSIS_DISPLAY_NAME ${SOSAGE_NAME})
set(CPACK_NSIS_PACKAGE_NAME ${SOSAGE_NAME})
set(CPACK_PACKAGE_INSTALL_DIRECTORY ${SOSAGE_EXE_NAME})
set(CPACK_NSIS_URL_INFO_ABOUT ${SOSAGE_URL})
set(CPACK_NSIS_MUI_ICON "${SOSAGE_DATA_FOLDER}/resources/icon.ico")
set(CPACK_NSIS_CONTACT ${SOSAGE_AUTHOR})
set(CPACK_NSIS_WELCOME_TITLE "Welcome to the installer of ${SOSAGE_NAME}")
set(CPACK_NSIS_FINISH_TITLE "The installation ${SOSAGE_NAME} is now finised")
set(CPACK_NSIS_UNINSTALL_NAME "uninstall-${SOSAGE_EXE_NAME}")
set(CPACK_NSIS_CREATE_ICONS_EXTRA
    "CreateShortCut '$SMPROGRAMS\\\\$STARTMENU_FOLDER\\\\${SOSAGE_NAME}.lnk' '$INSTDIR\\\\${SOSAGE_EXE_NAME}.exe'"
)
