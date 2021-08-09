/*
  [include/Sosage/Utils/gamepad_labels.h]
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

#ifndef SOSAGE_UTILS_GAMEPAD_LABELS_H
#define SOSAGE_UTILS_GAMEPAD_LABELS_H

#include <Sosage/Utils/enum.h>

#include <string>
#include <unordered_map>

namespace Sosage
{

std::string gamepad_label (const Gamepad_type& type, const Event_value& value)
{
  static std::unordered_map<Event_value, std::string>
      japan = { {NORTH, "X"}, {EAST, "A"}, {SOUTH, "B"}, {WEST, "Y"} };
  static std::unordered_map<Event_value, std::string>
      usa = { {NORTH, "Y"}, {EAST, "B"}, {SOUTH, "A"}, {WEST, "X"} };
  static std::unordered_map<Event_value, std::string>
      keyboard = { {NORTH, "I"}, {EAST, "L"}, {SOUTH, "K"}, {WEST, "J"} };

  if (type == JAPAN) return japan[value];
  if (type == USA) return usa[value];
  if (type == KEYBOARD_PAD) return keyboard[value];

  // no label
  return "";
}

}

#endif // SOSAGE_UTILS_GAMEPAD_LABELS_H
