/*
  [include/Sosage/Config.h]
  Constant and variable configuration values (world size, FPS, etc.).

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#ifndef SOSAGE_CONFIG_H
#define SOSAGE_CONFIG_H

#include <Sosage/platform.h>
#include <memory>

namespace Sosage
{

#ifdef SOSAGE_WINDOWS
constexpr char folder_separator = '\\';
#else
constexpr char folder_separator = '/';
#endif

constexpr int world_width = 1920;
constexpr int world_height = 1000;
constexpr int world_depth = 3240;

constexpr int interface_depth = 1000000;
constexpr int inventory_back_depth = interface_depth + 1;
constexpr int inventory_front_depth = inventory_back_depth + 1;
constexpr int inventory_over_depth = inventory_front_depth + 1;
constexpr int cursor_depth = inventory_over_depth + 1;

constexpr double boundary_precision = 2.0;

constexpr int gui_fps = 60;
constexpr int animation_fps = 12;

constexpr int character_speed = 34;

constexpr int text_outline = 10;
constexpr int displayed_inventory_size = 4;

constexpr double button_click_duration = 0.1;


struct Config
{
  int interface_width;
  int interface_height;
  int window_width;
  int window_height;

  int characters_per_second;

  bool fullscreen;

  Config (int window_width = 1200, bool fullscreen = false)
    : interface_width (0)
    , interface_height (200)
    , window_width (window_width)
    , window_height (window_width * 10 / 16)
    , characters_per_second (12)
    , fullscreen (fullscreen)
  { }

};

#ifdef SOSAGE_ANDROID
inline Config& config (int window_width = 1600, bool fullscreen = true)
#else
inline Config& config (int window_width = 1600, bool fullscreen = false)
#endif
{
  static std::unique_ptr<Config> c;
  if (c == std::unique_ptr<Config>())
    c = std::unique_ptr<Config>(new Config(window_width, fullscreen));
  return *c;
}

} // namespace Sosage

#endif // SOSAGE_CONFIG_H
