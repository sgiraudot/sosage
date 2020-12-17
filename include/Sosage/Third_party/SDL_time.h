/*
  [include/Sosage/Third_party/SDL_time.h]
  Wrapper for SDL library (time handling).

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

#ifndef SOSAGE_THIRD_PARTY_SDL_TIME_H
#define SOSAGE_THIRD_PARTY_SDL_TIME_H

#include <Sosage/Utils/error.h>
#include <Sosage/Config/config.h>

#include <SDL_timer.h>

namespace Sosage::Third_party
{

class SDL_time
{
public:

  using Unit = Uint32;
  using Duration = Uint32;

  static Unit now()
  {
    Unit out = SDL_GetTicks();
    debug("Ticks = " + std::to_string(out));
    return out;
  }

  static void wait (const Duration& d) { SDL_Delay (d); }
};

} // namespace Sosage::Third_party

#endif // SOSAGE_THIRD_PARTY_SDL_TIME_H
