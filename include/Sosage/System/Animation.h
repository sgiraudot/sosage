/*
  [include/Sosage/System/Animation.h]
  Generate animations and handle frame-by-frame image selection.

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

#ifndef SOSAGE_SYSTEM_ANIMATION_H
#define SOSAGE_SYSTEM_ANIMATION_H

#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Config/config.h>
#include <Sosage/Content.h>
#include <Sosage/System/Base.h>

namespace Sosage
{

namespace Config
{
constexpr int camera_limit_left = world_width / 4;
constexpr int camera_limit_right = (3 * world_width) / 4;
constexpr int character_speed = 34;
} // namespace Config

namespace System
{

class Animation : public Base
{
private:

  std::size_t m_frame_id;
  std::vector<Component::Handle> m_to_remove;

  std::unordered_set<std::string> m_just_started;

public:

  Animation (Content& content);

  virtual void run();

  bool run_loading();

  void place_and_scale_character (const std::string& id);
  void generate_random_idle_animation (const std::string& id, bool looking_right);
  void generate_random_idle_head_animation (const std::string& id, bool looking_right);
  void generate_random_idle_body_animation  (const std::string& id, bool looking_right);

private:

  void run_gui_frame();
  void run_animation_frame();

  void handle_character_lookat (bool in_new_room);
  void handle_animation_stops();
  bool handle_moves();
  void handle_animation_starts();
  void handle_state_changes();
  void handle_characters_headmove (Component::Animation_handle anim);
  void trigger_step_sounds (Component::Animation_handle anim);

  bool compute_movement_from_path (Component::Path_handle path);
  void set_move_animation (const std::string& id, const Vector& direction);

  void generate_random_mouth_animation (const std::string& id);
  void generate_animation (const std::string& id, const std::string& anim);

  bool fade (double begin_time, double end_time, bool fadein);

  void update_camera_target();
};

} // namespace System

} // namespace Sosage

#endif // SOSAGE_SYSTEM_GRAPHIC_H
