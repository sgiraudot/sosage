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

namespace Config
{
constexpr int gui_fps = 60;
} // namespace Config

using namespace Core;

class Clock
{
  Time::Unit m_latest;
  Time::Duration m_refresh_time;

  double m_mean;
  double m_active;
  std::size_t m_nb;

  std::size_t m_nb_recorded;
  std::size_t m_nb_missed;

  double m_fps;
  double m_cpu;

  Time::Unit m_start;
  double m_time;

  Clock (const Clock&);

public:

  Clock();
  void update();
  bool wait(bool verbose);
  double fps() const;
  double cpu() const;
  double time() const;

private:

  // 60fps might be too greedy for some configs,
  // adapt if too many missed frames
  void adapt_fps(bool missed);
};

std::size_t frame_id (const double& time);
double frame_time (const double& time);

} // namespace Sosage

#endif // SOSAGE_UTILS_CLOCK_H
