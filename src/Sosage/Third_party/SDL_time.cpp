/*
  [src/Sosage/Third_party/SDL_time.cpp]
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

#include <Sosage/Third_party/SDL_time.h>
#include <Sosage/Utils/error.h>


namespace Sosage::Third_party
{

SDL_time::Unit SDL_time::now()
{
  Unit out = SDL_GetTicks();
  debug << "Ticks = " << out << std::endl;
  return out;
}

void SDL_time::wait (const Duration& d)
{
  SDL_Delay (d);
}

} // namespace Sosage::Third_party
