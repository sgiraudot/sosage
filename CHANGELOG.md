# SOSAGE Changelog

History of versions and modifications of the SOSAGE engine.

## `v0.3.0 (xmas2020demo)` - 2020-12-25

### Release Management

- Cross-compilation Gnunux to Windows
- Script to generate all platforms installers
- Version handling by CMake

### Interface and Menus

- Remove threads to avoid problems, loading spinning image handled
  with callbacks instead
- Saving/loading
- Main menu

### Game Engine

- Cutscenes
- Indices

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
