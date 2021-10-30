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

#include <Sosage/Config/options.h>
#include <Sosage/Core/Time.h>


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

#ifdef SOSAGE_PROFILE_FINELY
#include <fstream>
#include <vector>
#endif

namespace Sosage
{
using namespace Core;

class Timer
{
  std::string m_id;
  Time::Unit m_start;

#ifdef SOSAGE_PROFILE_FINELY
  std::vector<Time::Duration> m_duration;
#else
  Time::Duration m_duration;
  unsigned int m_nb = 0;
#endif

  bool m_master;

public:

  Timer (const std::string& id, bool master = true);
  ~Timer();
  void start();
  void stop();
  void display();
#ifndef SOSAGE_PROFILE_FINELY
  double mean_duration() const;
#endif
  std::string to_string (double d) const;
};

class Counter
{
  std::string m_id;
  unsigned int m_nb;

public:

  Counter (const std::string& id);
  ~Counter ();
  void increment();
};

}

#endif // SOSAGE_UTILS_PROFILING_H


