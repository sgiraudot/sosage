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
  : m_mean(0), m_nb(0), m_time(0)
{
  m_start = Time::now();
  m_latest = m_start;
}

void Clock::update(bool verbose)
{
  Time::Unit now = Time::now();
  if (verbose)
  {
    m_mean += (now - m_latest);
    ++ m_nb;
    if (m_nb == 20)
    {
      m_fps = 1000. / (m_mean / m_nb);
      m_mean = 0.;
      m_nb = 0;
    }
  }
  m_latest = now;
  m_time = (m_latest - m_start) / 1000.;
}

double Clock::fps() const
{
  return m_fps;
}

double Clock::time() const
{
  return m_time;
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
