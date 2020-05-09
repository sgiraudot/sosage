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

namespace Sosage
{
using namespace Core;

class Timer
{
  std::string m_id;
  Time::Unit m_start;
  Time::Duration m_duration;
  unsigned int m_nb;
  
public:

  Timer (const std::string& id) : m_id (id), m_nb(0) { }

  ~Timer()
  {
    output("[" + m_id + " profiling] " + to_string(m_duration) + " ("
           + to_string(mean_duration()) + " per iteration)");
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

  std::string to_string (double d)
  {
    if (d < 100)
      return std::to_string(d) + "ms";
    return std::to_string(d / 1000.) + "s";
  }
};


}


#endif // SOSAGE_UTILS_PROFILING_H


