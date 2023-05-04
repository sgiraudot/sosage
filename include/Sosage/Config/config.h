/*
  [include/Sosage/Config/config.h]
  Constant shared configuration values (world size, FPS, etc.).

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

#include <memory>
#include <limits>

namespace Sosage::Config
{

constexpr int world_width = 1920;
constexpr int world_height = 1080;

constexpr int inventory_height = 150;
constexpr int inventory_active_zone = 50;

constexpr int object_reach_x = 250;
constexpr int object_reach_y = 100;
constexpr int object_reach_hysteresis = 20;

constexpr double minimum_reaction_time = 0.25;

constexpr double camera_speed = 1.0;

constexpr int animation_fps = 12;

constexpr auto possible_actions = { "look", "move", "take", "inventory", "use", "combine", "goto" };

constexpr auto save_ids = { "auto", "1", "2", "3", "4", "5" };

constexpr double default_sound_fade_time = 0.2;

extern double interface_scale;

enum Depth
{
  interface_depth = 1000000,
  inventory_depth,
  label_depth,
  action_button_depth,
  dialog_depth,
  overlay_depth,
  menu_front_depth,
  menu_button_depth,
  menu_text_depth,
  cursor_depth,
  notification_depth,
  loading_depth
};

enum Dialog_speed
{
  SLOW = 18,
  MEDIUM_SPEED = 12,
  FAST = 9
};

enum Interface_scale
{
  TINY = 4,
  SMALL = 6,
  LARGE = 9,
  HUGE_ = 12
  /* Old dialog sizes =
  SMALL = 9,
  MEDIUM = 12,
  LARGE = 18,
  */
};

} // namespace Sosage::Config

#endif // SOSAGE_CONFIG_CONFIG_H
