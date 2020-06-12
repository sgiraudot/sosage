/*
  [include/Sosage/Utils/profiling.h]
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

#ifndef SOSAGE_UTILS_PROFILING_H
#define SOSAGE_UTILS_PROFILING_H

#include <Sosage/Core/Time.h>

#include <algorithm>

#ifdef SOSAGE_PROFILE
#  define SOSAGE_TIMER_START(x) static Timer x(#x); x.start()
#  define SOSAGE_TIMER_RESTART(x) x.start()
#  define SOSAGE_TIMER_STOP(x) x.stop()
#  define SOSAGE_COUNT(x) static Counter x(#x); x.increment()
#else
#  define SOSAGE_TIMER_START(x)
#  define SOSAGE_TIMER_RESTART(x)
#  define SOSAGE_TIMER_STOP(x)
#  define SOSAGE_COUNT(x)
#endif

namespace Sosage
{
using namespace Core;

class Timer
{
  std::string m_id;
  Time::Unit m_start;
  Time::Duration m_duration;
  unsigned int m_nb;
  bool m_master;

public:

  Timer (const std::string& id, bool master = true) : m_id (id), m_nb(0), m_master(master) { }

  ~Timer()
  {
    if (m_master)
    {
      std::cerr << "[Profiling " << m_id << "] ";
      display();
      std::cerr << std::endl;
    }
  }

  void start()
  {
    m_start = Time::now();
    ++ m_nb;
  }


  void stop()
  {
    m_duration += Time::now() - m_start;
  }

  double mean_duration() const { return m_duration / double(m_nb); }

  void display() const
  {
    std::cerr << to_string(m_duration);
    if (m_nb > 1)
      std::cerr << " ("
                << to_string(mean_duration())
                << " per iteration, " << m_nb << " iterations)";
  }

  std::string to_string (double d) const
  {
    if (d < 900)
      return std::to_string(std::round(d * 100) / 100).substr(0,4) + "ms";
    return std::to_string(std::round((d / 1000.) * 100) / 100).substr(0,4) + "s";
  }
};

class Counter
{
  std::string m_id;
  unsigned int m_nb;

public:

  Counter (const std::string& id) : m_id (id), m_nb(0) { }
  ~Counter ()
  {
    std::cerr << "[Profiling " << m_id << "] "
              << m_nb << " iteration(s)" << std::endl;
  }

  void increment() { ++ m_nb; }
};

}


#endif // SOSAGE_UTILS_PROFILING_H


