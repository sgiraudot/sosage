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

namespace Sosage::System
{

class Animation
{
private:

  Content& m_content;

public:

  Animation (Content& content);

  void run();
  
  void generate_random_idle_animation (Component::Animation_handle image,
                                       Component::Animation_handle head,
                                       Component::Animation_handle mouth,
                                       const Vector& direction);

  void generate_random_idle_head_animation (Component::Animation_handle head,
                                            Component::Animation_handle mouth,
                                            const Vector& direction);
  void generate_random_idle_body_animation (Component::Animation_handle body,
                                            const Vector& direction);

private:
  
  void compute_movement_from_path (Component::Path_handle path);
  void set_move_animation (Component::Animation_handle image,
                           Component::Animation_handle head,
                           Component::Animation_handle mouth,
                           const Vector& direction);

  void generate_random_mouth_animation (Component::Animation_handle mouth);
  void generate_pick_animation (Component::Animation_handle body);

  void update_camera_target();
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_GRAPHIC_H
