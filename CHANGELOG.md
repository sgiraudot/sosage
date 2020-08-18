# SOSAGE Changelog

History of versions and modifications of the SOSAGE engine.

## `dev`

### Release Management

- Basic script for generating AppImage
- Support for Emscripten / running in web browser
- Fix install path using relative resolution with `SDL_GetBasePath()`
- Introduction of testing using CTest

### Interface and Menus

- Handling configuration file and persistent settings
- Switch fullscreen on/off
- Loading screen

### Game Engine

- Much better path finder: 30x faster (0.5ms instead of 15ms)
- Resource manager to avoid loading N times the same file
- Moving camera for horizontal scrolling
- Animated objects
- Room changing while keeping inventory, etc.
- Integer objects + triggers
- Default action with right click

### Content

- Second room in progress

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
- First room (bureau) working entirely

### Content

- Full first room (bureau), although sounds are still from stock (this
  will be changed) and most graphics are missing shading

## `beginning` - 2019-10-24

- Nothing yet :)
