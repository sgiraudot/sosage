/*
  [include/Sosage/Config/config.h]
  Constant configuration values (world size, FPS, etc.).

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

#ifndef SOSAGE_CONFIG_CONFIG_H
#define SOSAGE_CONFIG_CONFIG_H

#include <Sosage/Config/platform.h>
#include <memory>

namespace Sosage
{

namespace Config
{

constexpr char folder_separator = (windows ? '\\' : '/');

constexpr int world_width = 1920;
constexpr int world_height = 1000;
constexpr int world_depth = 3240;

constexpr int camera_limit_left = world_width / 4;
constexpr int camera_limit_right = (3 * world_width) / 4;
constexpr double camera_speed = 0.1;

constexpr double char_spoken_time = 0.05;
constexpr double min_reading_time = 1.5;

constexpr double key_repeat_delay = 5.;

constexpr double boundary_precision = 2.0;

constexpr int gui_fps = 60;
constexpr int animation_fps = 12;

constexpr int character_speed = 34;

constexpr int text_outline = 10;
constexpr int displayed_inventory_size = 4;

constexpr double button_click_duration = 0.1;
constexpr double virtual_cursor_sensitivity = 5;
constexpr double virtual_cursor_speed = 2.0;

constexpr int max_music_volume = 128;

enum Depth
{
  interface_depth = 1000000,
  inventory_back_depth,
  inventory_front_depth,
  inventory_over_depth,
  dialog_depth,
  menu_back_depth,
  menu_front_depth,
  menu_button_depth,
  menu_text_depth,
  cursor_depth,
  overlay_depth,
  loading_depth
};

enum Layout
{
  AUTO,
  WIDESCREEN,
  STANDARD,
  SQUARE,
  PORTRAIT
};

} // namespace Config

} // namespace Sosage

#endif // SOSAGE_CONFIG_CONFIG_H
