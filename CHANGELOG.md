# SOSAGE Changelog

History of versions and modifications of the SOSAGE engine.

## `v1.2.0 (demo)` - TBD

## `v1.1.0 (modern-ui)` - 2021-10-05

### Release Management

- Integration of LZ4 library
- Integrate Android/Emscripten to global CMakeLists
- Data validation script

### Interface and Menus

- Fully reworked interface
- Unique aspect ratio 16/9
- Screen is 100% in-game, no permanent interface
- Support for gamepads and keyboard input
- Reworked support for touchscreens
- Fully reworked menus

### Game Engine

- SCAP program (Sosage Compressed Asset Packager)
- Handle packaged data with compressed custom image formats
- Precompute ground map borders
- I18n support

### Bug fixes

- Music volume was not taken into account when starting a new music

## `v1.0.0 (xmas2020demo)` - 2020-12-25

### Release Management

- Cross-compilation Gnunux to Windows
- Script to generate all platforms installers
- Version handling by CMake

### Interface and Menus

- Remove threads to avoid problems, loading spinning image handled
  with callbacks instead
- Saving/loading
- Main menu
- Settings menu
- Cursor choice menu

### Game Engine

- Cutscenes
- Hints

## `v0.2.0 (alpha2)` - 2020-11-11

### Release Management

- Basic script for generating AppImage
- Support for Emscripten / running in web browser
- CPack support for DEB/RPM packaging
- Fix install path using relative resolution with `SDL_GetBasePath()`
- Introduction of testing using CTest

### Interface and Menus

- Handling configuration file and persistent settings
- Switch fullscreen on/off
- Threaded animated loading screen

### Game Engine

- Much better path finder: 30x faster (0.5ms instead of 15ms)
- Resource manager to avoid loading N times the same file
- Moving camera for horizontal scrolling
- Animated objects
- Room changing while keeping inventory, etc.
- Integer objects + triggers
- Default action with right click
- Dialogs
- Sound effects
- Triggers
- Pretty much all game mechanisms

## `v0.1.0 (alpha1)` - 2020-04-25

### Release Management

- Support for GNU/Linux and Android
- Install procedure for GNU/Linux
- Icon and Freedesktop desktop file

### Interface and Menus

- Semi-responsive interface, either auto or manually selected

### Game Engine

- Most of the game mechanisms except for interactions with NPC
  (dialogs, giving objects, etc.) and changing room

## `beginning` - 2019-10-20

- Nothing yet :)
