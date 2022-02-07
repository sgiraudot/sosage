/*
  [include/Sosage/Core/Time.h]
  Abstraction file for third party library handling time.

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

#ifndef SOSAGE_CORE_TIME_H
#define SOSAGE_CORE_TIME_H

#include <Sosage/Third_party/SDL_time.h>

#include <chrono>
#include <thread>

namespace Sosage::Core
{

#ifdef SOSAGE_SDL_TIME
using Time = Third_party::SDL_time;
#else
class Time
{
public:

  using Unit = std::chrono::steady_clock::time_point::rep;
  using Duration = std::chrono::steady_clock::duration::rep;

  static Unit now()
  {
    return std::chrono::duration_cast<std::chrono::milliseconds>
        (std::chrono::steady_clock::now().time_since_epoch()).count();
  }

  static void wait (const Duration& d)
  {
    std::this_thread::sleep_for
        (std::chrono::steady_clock::duration
         (std::chrono::milliseconds { d }));
  }
};
#endif
} // namespace Sosage::Core

#endif
