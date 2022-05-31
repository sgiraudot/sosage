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
#include <Sosage/Utils/profiling.h>

namespace Sosage::System
{

namespace C = Component;

Time::Time (Content& content)
  : Base (content)
{
  set_fac<C::Debug>(GAME__DEBUG, "Game", "debug", m_content, m_clock);
  set_fac<C::Double> (CLOCK__TIME, "Clock", "time", 0.);
  set_fac<C::Double> (CLOCK__LATEST_ACTIVE, "Clock", "latest_active", 0.);
}

void Time::run()
{
  SOSAGE_TIMER_START(System_Time__run);
  SOSAGE_UPDATE_DBG_LOCATION("Time::run()");

  bool frame_under_refresh_time = m_clock.wait(true);
  SOSAGE_COUNT(Frames);
  if (!frame_under_refresh_time)
  {
    SOSAGE_COUNT(Frames_exceeding_refresh_time);
  }

  get<C::Double> (CLOCK__TIME)->set(m_clock.time());
  SOSAGE_TIMER_STOP(System_Time__run);
}

void Time::run_loading()
{
  m_clock.update();
  get<C::Double> (CLOCK__TIME)->set(m_clock.time());
}

} // namespace Sosage::System
