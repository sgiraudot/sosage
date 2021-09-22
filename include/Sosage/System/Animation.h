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
#include <Sosage/Content.h>
#include <Sosage/System/Handle.h>

namespace Sosage::System
{

class Animation : public Base
{
private:

  std::size_t m_frame_id;
  bool m_fade_to_remove;

public:

  Animation (Content& content);

  virtual void run();

  bool run_loading();

  void place_and_scale_character (const std::string& id, bool looking_right);
  void generate_random_idle_animation (const std::string& id, bool looking_right);
  void generate_random_idle_head_animation (const std::string& id, bool looking_right);
  void generate_random_idle_body_animation  (const std::string& id, bool looking_right);

private:

  void run_gui_frame();
  void run_animation_frame();

  bool compute_movement_from_path (Component::Path_handle path);
  void set_move_animation (const std::string& id, const Vector& direction);

  void generate_random_mouth_animation (const std::string& id);
  void generate_animation (const std::string& id, const std::string& anim);

  void fade (double begin_time, double end_time, bool fadein);

  double smooth_function(double xbegin, double xend, double ybegin, double yend, double x) const;
  void update_camera();
  void update_camera_target();
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_GRAPHIC_H
