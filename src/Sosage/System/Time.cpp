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

  if (signal ("Time", "speedup"))
  {
    auto begin_speedup = get_or_set<C::Double>("Time", "begin_speedup", m_clock.time());
    double time = begin_speedup->value()
                  + Config::speedup_factor * (m_clock.time()
                                              - begin_speedup->value());
    get<C::Double> (CLOCK__TIME)->set(time);
  }
  else if (auto begin_speedup = request<C::Double>("Time", "begin_speedup"))
  {
    double time = begin_speedup->value()
                  + Config::speedup_factor * (m_clock.time()
                                              - begin_speedup->value());

    // Avoid messing with the game time (sped up time doesn't count double)
    double real_time = m_clock.time();
    auto dtime = get<C::Double> (CLOCK__DISCOUNTED_TIME);
    dtime->set (dtime->value() + (time - real_time));

    m_clock.set (time);
    remove (begin_speedup);
    get<C::Double> (CLOCK__TIME)->set(time);
  }

  // Do not count time spent in menu for in-game time computation
  if (!signal("Game", "save") &&
      (status()->is(IN_MENU, PAUSED) || request<C::String>("Game", "new_room")))
  {
    if (signal("Game", "reset"))
    {
      get<C::Double>(CLOCK__DISCOUNTED_TIME)->set(m_clock.time());
      get<C::Double>(CLOCK__SAVED_TIME)->set(0);
    }

    get_or_set<C::Double>("Time", "in_menu_start", m_clock.time());
  }
  else if (auto start = request<C::Double>("Time", "in_menu_start"))
  {
    auto in_menu = get<C::Double> (CLOCK__DISCOUNTED_TIME);
    in_menu->set (in_menu->value() + (m_clock.time() - start->value()));
    remove (start);
  }
  SOSAGE_TIMER_STOP(System_Time__run);
}

} // namespace Sosage::System
