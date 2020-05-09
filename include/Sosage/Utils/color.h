/*
  [include/Sosage/Utils/color.h]
  Handle RGB colors with strings and hex values.

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

#ifndef SOSAGE_UTILS_COLOR_H
#define SOSAGE_UTILS_COLOR_H

#include <array>
#include <sstream>
#include <string>

namespace Sosage
{

inline std::array<unsigned char, 3> color_from_string (const std::string& str)
{
  std::stringstream ss(str);
  int num;
  ss >> std::hex >> num;
  return { (unsigned char)(num / 0x10000),
           (unsigned char)((num / 0x100) % 0x100),
           (unsigned char)(num % 0x100) };
}


} // namespace Sosage

#endif // SOSAGE_UTILS_COLOR_H
