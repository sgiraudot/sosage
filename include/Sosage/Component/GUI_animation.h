/*
  [include/Sosage/Component/GUI_animation.h]
  Animate GUI elements in a smooth way.

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

#ifndef SOSAGE_COMPONENT_GUI_ANIMATION_H
#define SOSAGE_COMPONENT_GUI_ANIMATION_H

#include <Sosage/Component/Base.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>

namespace Sosage::Component
{

class GUI_animation : public Base
{
  double m_start_time;
  double m_end_time;
  bool m_remove_after;

public:

  GUI_animation (const std::string& id, double start_time, double end_time,
                 bool remove_after);
  bool update (double current_time);
  bool remove_after() const;
  virtual void cancel () = 0;
  virtual void finalize () = 0;
  virtual void update_impl (double current_time) = 0;
  virtual const std::string& object_id() = 0;

protected:

  double smooth_function(double vstart, double vend, double t) const;
};

using GUI_animation_handle = std::shared_ptr<GUI_animation>;

class GUI_position_animation : public GUI_animation
{
  Position_handle m_position;
  Point m_start_pos;
  Point m_end_pos;

public:

  GUI_position_animation (const std::string& id, double start_time, double end_time,
                          Position_handle position, Point target, bool remove_after = false);
  void update (const Point& point);
  virtual void cancel();
  virtual void finalize();
  virtual void update_impl (double current_time);
  virtual const std::string& object_id();
};

using GUI_position_animation_handle = std::shared_ptr<GUI_position_animation>;

class GUI_image_animation : public GUI_animation
{
  Image_handle m_image;
  double m_start_scale;
  double m_end_scale;
  unsigned char m_start_alpha;
  unsigned char m_end_alpha;

public:

  GUI_image_animation (const std::string& id, double start_time, double end_time,
                       Image_handle image, double start_scale, double end_scale,
                       unsigned char start_alpha, unsigned char end_alpha,
                       bool remove_after = false);
  virtual void cancel();
  virtual void finalize();
  virtual void update_impl (double current_time);
  virtual const std::string& object_id();
};

using GUI_image_animation_handle = std::shared_ptr<GUI_image_animation>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GUI_ANIMATION_H
