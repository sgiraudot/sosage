/*
  [include/Sosage/Utils/random.h]
  Generates pseudo-random numbers

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

#ifndef SOSAGE_UTILS_RANDOM_H
#define SOSAGE_UTILS_RANDOM_H

namespace Sosage
{

inline int random_int (int min, int max)
{
  return min + (rand() % (max - min));
}

} // namespace Sosage

#endif // SOSAGE_UTILS_RANDOM_H
