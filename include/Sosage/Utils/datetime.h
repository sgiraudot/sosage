/*
  [include/Sosage/Utils/datetime.h]
  Useful date/time tools.

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

#ifndef SOSAGE_UTILS_DATETIME_H
#define SOSAGE_UTILS_DATETIME_H

#include <string>

namespace Sosage
{

std::string time_to_string (double time);
std::string date_to_string (int date);

} // namespace Sosage

#endif // SOSAGE_UTILS_DATETIME_H
