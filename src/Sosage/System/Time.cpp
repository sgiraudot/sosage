/*
  [src/Sosage/System/Time.cpp]
  Time handling (framerate, sleep, etc.).

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


#include <Sosage/Component/Debug.h>
#include <Sosage/System/Time.h>

namespace Sosage::System
{

Time::Time (Content& content)
  : m_content (content)
{
  m_content.set_fac<Component::Debug>(GAME__DEBUG, "game:debug", m_content, m_clock);
  m_content.set_fac<Component::Int> (CLOCK__FRAME_ID, "clock:frame_id", 0);
  m_content.set_fac<Component::Double> (CLOCK__FRAME_TIME, "clock:frame_time", 0.);
}

void Time::run()
{
  if constexpr (!Config::emscripten)
    m_clock.wait(true);

  m_content.get<Component::Int> (CLOCK__FRAME_ID)->set(m_clock.frame_id());
  m_content.get<Component::Double> (CLOCK__FRAME_TIME)->set(m_clock.frame_time());
}

} // namespace Sosage::System
