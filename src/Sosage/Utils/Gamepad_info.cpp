/*
  [src/Sosage/Utils/Gamepad_info.cpp]
  Handle different types of Gamepad.

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

#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/Gamepad_info.h>

#include <unordered_map>

namespace Sosage
{

Gamepad_info::Gamepad_info (unsigned short vendor, unsigned short product,
                            const std::string& name)
  : name (name)
{
  id = to_string (vendor) + ":" + to_string(product);
  if (contains (name, "Xbox") ||
      contains (name, "Steam"))
  {
    labels = XBOX;
    ok_down = true;
  }
  else if (startswith(name, "Nintendo") ||
           startswith(name, "8BitDo"))
  {
    labels = NINTENDO;
    ok_down = false;
  }
  else
  {
    labels = NO_LABEL;
    ok_down = true;
  }
}

Gamepad_info::Gamepad_info (const std::string& id, const Gamepad_labels& labels, const bool& ok_down)
  : id(id)
  , name ("Saved controller")
  , labels(labels)
  , ok_down(ok_down)
{

}

std::string gamepad_label (const Gamepad_info& info, const Event_value& value)
{
  static std::unordered_map<Event_value, std::string>
      nintendo = { {NORTH, "X"}, {EAST, "A"}, {SOUTH, "B"}, {WEST, "Y"} };
  static std::unordered_map<Event_value, std::string>
      xbox = { {NORTH, "Y"}, {EAST, "B"}, {SOUTH, "A"}, {WEST, "X"} };

  if (info.labels == NINTENDO)
    return nintendo[value];

  if (info.labels == XBOX)
    return xbox[value];

  // no label
  return "";
}

} // namespace Sosage
