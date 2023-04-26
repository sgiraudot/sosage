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
  , m_loading (false)
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

  if (!m_loading)
    limit_fps();

  double t = m_clock.time();

  if (m_loading)
    m_loading = false;

  // Never speed up cutscenes
  if (status()->is(CUTSCENE))
    receive ("Time", "speedup");

  if (signal ("Time", "speedup"))
  {
    auto begin_speedup = get_or_set<C::Double>("Time", "begin_speedup", m_clock.time());
    double time = begin_speedup->value()
                  + Config::speedup_factor * (m_clock.time()
                                              - begin_speedup->value());
    t = time;
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
    t = time;
  }

  if (status()->is(PAUSED))
  {
    auto begin_pause = get_or_set<C::Double>("Time", "begin_pause", m_clock.time());
    return;
  }
  else if (auto begin_pause = request<C::Double>("Time", "begin_pause"))
  {
    double time = begin_pause->value();
    double real_time = m_clock.time();
    debug << "Time spent in pause = " << real_time - time << std::endl;
    m_clock.set (time);
    m_clock.update();
    remove (begin_pause);
    t = m_clock.time();
  }
  get<C::Double> (CLOCK__TIME)->set(t);

  if (signal("Game", "reset"))
  {
    get<C::Double>(CLOCK__DISCOUNTED_TIME)->set(m_clock.time());
    get<C::Double>(CLOCK__SAVED_TIME)->set(0);
  }

  // Do not count time spent in menu for in-game time computation
  if (!signal("Game", "save") &&
      (status()->is(IN_MENU) || request<C::String>("Game", "new_room")))
  {

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

void Time::limit_fps()
{
  static bool needs_limits = false;
  static bool game_started = false;
  static bool needs_update = true;
  static double latest_refresh = 0.;
  static double refresh_time = 0.;
  static int nb_refresh = 0;
  if (needs_update)
  {
    if (!game_started)
    {
      if (signal ("Game", "new_room_loaded"))
      {
        latest_refresh = m_clock.time();
        refresh_time = 0;
        nb_refresh = 0;
        game_started = true;
      }
      return;
    }

    if (m_clock.time() == 0)
      return;

    refresh_time += (m_clock.time() - latest_refresh);
    nb_refresh ++;
    latest_refresh = m_clock.time();

    if (refresh_time > 5)
    {
      refresh_time /= nb_refresh;
      double estimated_fps = 1. / refresh_time;
      if (estimated_fps > 70)
      {
        debug << "FPS = " << estimated_fps << ", activating limit to 60 FPS" << std::endl;
        needs_limits = true;
      }
      else
      {
        debug << "FPS = " << estimated_fps << ", no limit needed" << std::endl;
      }
      needs_update = false;
    }
  }

  if (needs_limits)
  {
    constexpr double refresh_time = (1. / 60);
    double time_spent = m_clock.time() - latest_refresh;
    if (time_spent < refresh_time)
      m_clock.sleep (refresh_time - time_spent);
    latest_refresh = m_clock.time();
  }
}

} // namespace Sosage::System
