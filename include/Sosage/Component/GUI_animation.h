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

#include <Sosage/Component/Handle.h>
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
                 bool remove_after)
    : Base(id), m_start_time (start_time), m_end_time(end_time), m_remove_after(remove_after)
  { }

  bool update (double current_time)
  {
    if (current_time >= m_end_time)
    {
      finalize();
      return false;
    }

    update_impl(current_time);
    return true;
  }

  bool remove_after() const { return m_remove_after; }

  virtual void finalize () = 0;
  virtual void update_impl (double current_time) = 0;
  virtual const std::string& object_id() = 0;

protected:

  double smooth_function(double vstart, double vend, double t) const
  {
    return vstart + (vend - vstart) * std::sqrt(std::sin((t-m_start_time)*M_PI
                                                         / (2 * (m_end_time-m_start_time))));
  }
};

using GUI_animation_handle = std::shared_ptr<GUI_animation>;

class GUI_position_animation : public GUI_animation
{
  Position_handle m_position;
  Point m_start_pos;
  Point m_end_pos;

public:

  GUI_position_animation (const std::string& id, double start_time, double end_time,
                          Position_handle position, Point target, bool remove_after = false)
    : GUI_animation(id, start_time, end_time, remove_after)
    , m_position(position)
    , m_start_pos(position->value()), m_end_pos(target)
  { }

  virtual void finalize()
  {
    m_position->set(m_end_pos);
  }

  virtual void update_impl (double current_time)
  {
    m_position->set (Point(smooth_function (m_start_pos.x(), m_end_pos.x(), current_time),
                           smooth_function (m_start_pos.y(), m_end_pos.y(), current_time)));
  }

  virtual const std::string& object_id() { return m_position->id(); }
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
                       bool remove_after = false)
    : GUI_animation(id, start_time, end_time, remove_after)
    , m_image (image)
    , m_start_scale (start_scale), m_end_scale (end_scale)
    , m_start_alpha (start_alpha), m_end_alpha (end_alpha)
  { }

  virtual void finalize()
  {
    m_image->set_scale (m_end_scale);
    m_image->set_alpha (m_end_alpha);
    m_image->set_highlight (0);
  }

  virtual void update_impl (double current_time)
  {
    m_image->set_scale(smooth_function (m_start_scale, m_end_scale, current_time));
    m_image->set_alpha(smooth_function (m_start_alpha, m_end_alpha, current_time));
    m_image->set_highlight (0);
  }

  virtual const std::string& object_id() { return m_image->id(); }
};

using GUI_image_animation_handle = std::shared_ptr<GUI_image_animation>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GUI_ANIMATION_H
