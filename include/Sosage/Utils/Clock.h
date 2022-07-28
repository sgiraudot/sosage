/*
  [include/Sosage/Utils/Clock.h]
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

#ifndef SOSAGE_UTILS_CLOCK_H
#define SOSAGE_UTILS_CLOCK_H

#include <Sosage/Core/Time.h>

namespace Sosage
{

using namespace Core;

class Clock
{
  Time::Unit m_latest;
  double m_mean;
  std::size_t m_nb;
  double m_fps;
  Time::Unit m_start;
  double m_time;

  Clock (const Clock&);

public:

  Clock();
  double get() const;
  void update(bool verbose = false);
  double fps() const;
  double time() const;

};

std::size_t frame_id (const double& time);
double frame_time (const double& time);

} // namespace Sosage

#endif // SOSAGE_UTILS_CLOCK_H
