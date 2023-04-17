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
  set_fac<C::Double> (CLOCK__DISCOUNTED_TIME, "Clock", "discounted_time", 0);
  set_fac<C::Double> (CLOCK__SAVED_TIME, "Clock", "saved_time", 0);
}

void Time::run()
{
  SOSAGE_TIMER_START(System_Time__run);
  SOSAGE_UPDATE_DBG_LOCATION("Time::run()");

  m_clock.update(true);
  get<C::Double> (CLOCK__TIME)->set(m_clock.time());

  if (auto begin_speedup = request<C::Double>("Speedup", "begin"))
  {
    double time = begin_speedup->value()
                  + Config::speedup_factor * (m_clock.time()
                                              - begin_speedup->value());
    get<C::Double> (CLOCK__TIME)->set(time);

    if (status()->is(CUTSCENE) || receive ("Time", "end_speedup"))
    {
      m_clock.set (time);
      remove (begin_speedup);
    }
  }

  if (receive ("Time", "begin_speedup") && !status()->is(CUTSCENE))
    set<C::Double>("Speedup", "begin", m_clock.time());

  // Do not count time spent in menu for in-game time computation
  if (!signal("Game", "save") &&
      (status()->is(IN_MENU, PAUSED) || request<C::String>("Game", "new_room")))
    get_or_set<C::Double>("Time", "in_menu_start", m_clock.time());
  else if (auto start = request<C::Double>("Time", "in_menu_start"))
  {
    auto in_menu = get<C::Double> (CLOCK__DISCOUNTED_TIME);
    in_menu->set (in_menu->value() + (m_clock.time() - start->value()));
    remove (start);
  }
  SOSAGE_TIMER_STOP(System_Time__run);
}

} // namespace Sosage::System
