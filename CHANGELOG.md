# SOSAGE Changelog

History of versions and modifications of the SOSAGE engine.

## `v1.4.3 (returnz3)` - 2023-05-16

### Bug fixes

- Fix wrong OK/Cancel buttons when solving a code

## `v1.4.2 (returnz2)` - 2023-05-15

### Bug fixes

- Fix Android AAB generation using Play Asset Delivery
- Fix bug in cutscene skipping (one step missed)

## `v1.4.1 (returnz1)` - 2023-05-13

### Bug fixes

- Fix notifications sometimes not displayed
- Fix Menu bug when coming from inventory
- Fix crash if savegame with outdated room
- Fix cutscene skipping lefting some moves unfinished
- Fix hidden characters sometimes reappearing
- Remove character's path if move:[] is called

## `v1.4.0 (returnz)` - 2023-05-11

### Release Management

- Integrate Steam API to properly detect language on Steam Deck
- Integrate data version in CMake
- Better version name and code for Android

### Interface and menus

- Controls are sumed up in menu
- Customize gamepad button styles and OK/Cancel orientation
- Support PlayStation button style
- Display a forbidden sign when trying to speed up cutscene
- End menu with options to start a new game or quit

### Bug fixes

- Prevent player from moving using DPAD when locked/cutscene
- Cutscene only skips until unlock now
- Fix look:[] function when target is different from object

## `v1.3.3 (superfluous3)` - 2023-05-09

### Bug fixes

- Fix locale badly detected if `fr_COUNTRY` (with `COUNTRY != FR`) or
  `en_COUNTRY` (with `COUNTRY != US`)
- Fix localized scenery not loaded if scenery has states

## `v1.3.2 (superfluous2)` - 2023-05-05

### Release Management

- Change license mechanism to use asset license

### Bug fixes

- Fix engine version not displayed correctly on menu
- Fix assert depackager

## `v1.3.1 (superfluous1)` - 2023-04-27

### Bug Fixes

- Add missing zoom:[] variant
- Saved dialog not reset after loading
- Fix time not correctly reset after game reset
- Memory error, replace free() by SDL_free()
- Remove right-clic to skip cutscene, too easy to do it unintentionnally
- States not changed if cutscene is skipped

## `v1.3.0 (superfluous)` - 2023-04-25

### Release Management

- Handle APK version number with CMake
- Include SDL Mixer Ext
- Automatic random test input selected at runtime
- Fix RGB transparent pixels
- Add demo mode
- Force load option in dev mode
- Clear separation of Release/Beta/Dev/Demo/etc.
- Parallelize data packager

### Interface and menus

- Wriggly interface
- Keep track of in-game time
- Autosave
- Multiple saves, no exit required
- Notification system
- Variable dialog size generalized to variable scale interface

### Game Engine

- Add possibility to only define highlight mask of object
- Fast forward mode
- Custom functions in YAML
- Multitrack musics
- Music positions is saved and reloaded when entering a room again
- Signals in YAML
- Multiple inventory states
- Randomized actions
- Global items
- Relative label positions
- Actions on characters
- Secondary ground map for NPCs
- Replace ONCE behavior in dialog by signals conditions
- Hint mechanism replaced by dialogs
- Pause animations / pause when window minimized
- Change state synced with animation FP

### Bug Fixes

- A shitload

## `v1.2.3 (summer2022demo3)` - 2022-08-31

### Bug Fixes

- Update Android framework to latest SDL version to fix AAB-compatibility

## `v1.2.2 (summer2022demo2)` - 2022-08-16

### Game Engine

- Remove CPU-handled FPS, let vsync deal with frame syncing/delay
- Segment surfaces/textures to avoid overly large memory usage
- Remove now useless texture downscale
- Use HC LZ4 compression
- Precompute highlights and masks

### Bug Fixes

- Use vsync to avoid artifacts in fullscreen
- Memory leaks on sound/music objects fixed

## `v1.2.1 (summer2022demo1)` - 2022-06-29

### Bug Fixes

- Blocked camera after cutscene is fixed

## `v1.2.0 (summer2022demo)` - 2022-06-27

### Release Management

- Global script to build for every platform
- Cross-compilation Gnunux to MacOS
- GUI-less mode (for auto-testing)
- Automatic random-input testing

### Interface and Menus

- Phone numbers
- Hint system integrated to the phone
- Head moves while character speaks

### Game Engine

- Fully reworked component storing to be more efficient, limiting string constructions
- Improved gamepad handling
- Simultaneous actions at the same time
- Camera zoom
- Cutscenes are now simple rooms with specific functions
- 60fps moves/scalings
- Possibility to skip dialogs
- Unified fade mechanism
- Subpixel rendering
- Stereo sound effects

### Bug Fixes

- Many many MANY bugs found from auto-random-input
- Adjacent edges in graphs were broken, they are fixed

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

### Bug Fixes

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
