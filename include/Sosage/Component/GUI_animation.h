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
  Point m_start_pos;
  Point m_end_pos;

  Position_handle m_position;

public:

  GUI_animation (const std::string& id, double start_time, double end_time,
                 Position_handle position, Point target)
    : Base(id), m_start_time (start_time), m_end_time(end_time)
    , m_start_pos(position->value()), m_end_pos(target)
    , m_position(position)
  { }

  bool update (double current_time)
  {
    if (current_time >= m_end_time)
    {
      m_position->set(m_end_pos);
      return false;
    }
    m_position->set (Point(smooth_function (m_start_pos.x(), m_end_pos.x(), current_time),
                           smooth_function (m_start_pos.y(), m_end_pos.y(), current_time)));

    return true;
  }

private:

  double smooth_function(double vstart, double vend, double t) const
  {
    return vstart + (vend - vstart) * std::sqrt(std::sin((t-m_start_time)*M_PI
                                                         / (2 * (m_end_time-m_start_time))));
  }
};

using GUI_animation_handle = std::shared_ptr<GUI_animation>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GUI_ANIMATION_H
