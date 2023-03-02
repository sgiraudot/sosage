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
#include <string>

namespace Sosage
{

using RGB_color = std::array<unsigned char, 3>;
using RGBA_color = std::array<unsigned char, 4>;
RGB_color color_from_string (const std::string& str);

bool is_transparent_black_or_white (const RGBA_color& color);

} // namespace Sosage

#endif // SOSAGE_UTILS_COLOR_H
