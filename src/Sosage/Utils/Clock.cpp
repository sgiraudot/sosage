/*
  [src/Sosage/Utils/Clock.cpp]
  Clocks and timers.

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

#include <Sosage/Config/config.h>
#include <Sosage/Utils/Clock.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage
{

Clock::Clock (const Clock&)
{ }

Clock::Clock()
  : m_refresh_time (1000. / double(Config::gui_fps))
  , m_mean(0), m_active(0), m_nb(0), m_nb_recorded(0),
    m_nb_missed(0), m_fps(Config::gui_fps), m_cpu(0)
  , m_time(0)
{
  m_start = Time::now();
  m_latest = m_start;
}

void Clock::update()
{
  m_time = (Time::now() - m_start) / 1000.;
  m_latest = m_time;
}

bool Clock::wait (bool verbose)
{
  Time::Unit now = Time::now();
  Time::Duration duration = now - m_latest;

  bool out = true;

  if (duration > m_refresh_time)
  {
    debug << "Warning: frame lasted " << duration << " (max is " << m_refresh_time << ")" << std::endl;
    out = false;
  }
  adapt_fps (duration > m_refresh_time);

  if constexpr (!Config::emscripten)
  {
    SOSAGE_TIMER_START(CPU_idle);
    if (duration < m_refresh_time)
      Time::wait (m_refresh_time - duration);
    SOSAGE_TIMER_STOP(CPU_idle);
  }

  now = Time::now();
  if (verbose)
  {
    m_active += duration;
    m_mean += (now - m_latest);
    ++ m_nb;
    if (m_nb == 20)
    {
      m_fps = 1000. / (m_mean / m_nb);
      m_cpu = 100. * (m_active / (m_nb * m_refresh_time));
      m_mean = 0.;
      m_active = 0.;
      m_nb = 0;
    }
  }
  m_latest = now;

  m_time = (m_latest - m_start) / 1000.;

  return out;
}

double Clock::fps() const
{
  return m_fps;
}

double Clock::cpu() const
{
  return m_cpu;
}

double Clock::time() const
{
  return m_time;
}

void Clock::adapt_fps (bool missed)
{
  static const std::size_t interval = 1000;
  static const std::size_t max_missed = 100;

  ++ m_nb_recorded;
  if (missed)
    ++ m_nb_missed;
  if (m_nb_recorded == interval)
  {
    debug << m_nb_missed / 10. << "% frames exceeded allowed refresh time" << std::endl;
    if (m_nb_missed > max_missed)
    {
      int current_fps = int(1000. / m_refresh_time);
      int target_fps = int(0.8 * current_fps);
      if (target_fps >= Config::animation_fps)
      {
        m_refresh_time = 1000. / double(target_fps);
        debug << "Downgrading FPS to " << int(1000. / m_refresh_time) << std::endl;
      }
      else
        debug << "Can't downgrade FPS anymore (" << int(1000. / m_refresh_time) << ")" << std::endl;
    }
    else
      debug << "Keeping FPS to " << int(1000. / m_refresh_time) << std::endl;


    m_nb_recorded = 0;
    m_nb_missed = 0;
  }
}

std::size_t frame_id (const double& time)
{
  return std::size_t(time * Config::animation_fps);
}

double frame_time (const double& time)
{
  return frame_id(time) / double(Config::animation_fps);
}

} // namespace Sosage
