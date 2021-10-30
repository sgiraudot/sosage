/*
  [src/Sosage/Component/GUI_animation.cpp]
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

#include <Sosage/Component/GUI_animation.h>

namespace Sosage::Component
{

GUI_animation::GUI_animation (const std::string& id, double start_time, double end_time,
                              bool remove_after)
  : Base(id), m_start_time (start_time), m_end_time(end_time), m_remove_after(remove_after)
{ }

bool GUI_animation::update (double current_time)
{
  if (current_time >= m_end_time)
  {
    finalize();
    return false;
  }

  update_impl(current_time);
  return true;
}

bool GUI_animation::remove_after() const
{
  return m_remove_after;
}

double GUI_animation::smooth_function(double vstart, double vend, double t) const
{
  return vstart + (vend - vstart) * std::sqrt(std::sin((t-m_start_time)*M_PI
                                                       / (2 * (m_end_time-m_start_time))));
}

GUI_position_animation::GUI_position_animation (const std::string& id, double start_time, double end_time,
                                                Position_handle position, Point target, bool remove_after)
  : GUI_animation(id, start_time, end_time, remove_after)
  , m_position(position)
  , m_start_pos(position->value()), m_end_pos(target)
{ }

void GUI_position_animation::update (const Point& point)
{
  if (m_end_pos == point)
    return;
  m_end_pos = point;
}

void GUI_position_animation::cancel()
{
  m_position->set(m_start_pos);
}

void GUI_position_animation::finalize()
{
  m_position->set(m_end_pos);
}

void GUI_position_animation::update_impl (double current_time)
{
  m_position->set (Point(smooth_function (m_start_pos.x(), m_end_pos.x(), current_time),
                         smooth_function (m_start_pos.y(), m_end_pos.y(), current_time)));
}

const std::string& GUI_position_animation::object_id()
{
  return m_position->id();
}

GUI_image_animation::GUI_image_animation (const std::string& id, double start_time, double end_time,
                                          Image_handle image, double start_scale, double end_scale,
                                          unsigned char start_alpha, unsigned char end_alpha,
                                          bool remove_after)
  : GUI_animation(id, start_time, end_time, remove_after)
  , m_image (image)
  , m_start_scale (start_scale), m_end_scale (end_scale)
  , m_start_alpha (start_alpha), m_end_alpha (end_alpha)
{ }

void GUI_image_animation::cancel()
{
  m_image->set_scale (m_start_scale);
  m_image->set_alpha (m_start_alpha);
  m_image->set_highlight (0);
}

void GUI_image_animation::finalize()
{
  m_image->set_scale (m_end_scale);
  m_image->set_alpha (m_end_alpha);
  m_image->set_highlight (0);
  if (m_end_alpha == 0)
    m_image->on() = false;
}

void GUI_image_animation::update_impl (double current_time)
{
  m_image->set_scale(smooth_function (m_start_scale, m_end_scale, current_time));
  m_image->set_alpha(smooth_function (m_start_alpha, m_end_alpha, current_time));
  m_image->set_highlight (0);
}

const std::string& GUI_image_animation::object_id()
{
  return m_image->id();
}

} // namespace Sosage::Component
