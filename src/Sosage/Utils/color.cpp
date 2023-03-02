/*
  [src/Sosage/Utils/color.cpp]
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

#include <Sosage/Utils/color.h>

#include <sstream>

namespace Sosage
{

RGB_color color_from_string (const std::string& str)
{
  std::stringstream ss(str);
  int num;
  ss >> std::hex >> num;
  return { (unsigned char)(num / 0x10000),
           (unsigned char)((num / 0x100) % 0x100),
           (unsigned char)(num % 0x100) };
}

bool is_transparent_black_or_white (const RGBA_color& color)
{
  if (color[3] != 0)
    return false;

  return (color[0] == 255 && color[1] == 255 && color[2] == 255)
      || (color[0] == 0 && color[1] == 0 && color[2] == 0);
}

} // namespace Sosage
