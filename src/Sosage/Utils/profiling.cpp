/*
  [src/Sosage/Utils/profiling.cpp]
  Tools for profiling code.

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

#include <Sosage/Utils/error.h>
#include <Sosage/Utils/profiling.h>

#include <algorithm>
#include <cmath>

namespace Sosage
{

Timer::Timer (const std::string& id, bool master)
  : m_id (id), m_master(master)
#ifndef SOSAGE_PROFILE_FINELY
, m_duration(0), m_nb(0)
#endif
{ }

Timer::~Timer()
{
  if (m_master)
  {
    debug << "[Profiling " << m_id << "] " << std::endl;
    display();
  }
}

void Timer::start()
{
  m_start = Time::now();
#ifndef SOSAGE_PROFILE_FINELY
  ++ m_nb;
#endif
}

void Timer::stop()
{
#ifdef SOSAGE_PROFILE_FINELY
  m_duration.push_back(Time::now() - m_start);
#else
  m_duration += Time::now() - m_start;
#endif
}

void Timer::display()
{
#ifdef SOSAGE_PROFILE_FINELY
  if (m_id == "CPU_idle")
  {
    std::ofstream ofile ("cpu_idle.plot");
    for (const auto& t : m_duration)
      ofile << t << std::endl;
  }

  Time::Duration total = 0;
  for (const auto& d : m_duration)
    total += d;
  std::sort(m_duration.begin(), m_duration.end());
  debug << "Min = " << to_string(m_duration.front())
      << ", 10% = " << to_string(m_duration[m_duration.size() / 10])
      << ", median = " << to_string(m_duration[m_duration.size() / 2])
      << ", 90% = " << to_string(m_duration[9 * m_duration.size() / 10])
      << ", max = " << to_string(m_duration.back())
      << ", total = " << to_string(total)
      << ", mean = " << to_string(total / m_duration.size()) << std::endl;
#else
  debug << m_duration
        << ((m_nb > 1)
            ? " (" + to_string(mean_duration()) + " per iteration, " + std::to_string(m_nb) + " iterations)"
           : "") << std::endl;
#endif
}

#ifndef SOSAGE_PROFILE_FINELY
double Timer::mean_duration() const
{
  return m_duration / double(m_nb);
}
#endif

std::string Timer::to_string (double d) const
{
  if (d < 900)
    return std::to_string(std::round(d * 100) / 100).substr(0,4) + "ms";
  return std::to_string(std::round((d / 1000.) * 100) / 100).substr(0,4) + "s";
}

Counter::Counter (const std::string& id)
  : m_id (id), m_nb(0)
{ }

Counter::~Counter ()
{
  debug << "[Profiling " << m_id << "] " << m_nb << " iteration(s)" << std::endl;
}

void Counter::increment()
{
  ++ m_nb;
}

} // namespace Sosage
