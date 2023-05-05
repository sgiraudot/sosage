/*
  [include/Sosage/Utils/Gamepad_info.h]
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

#ifndef SOSAGE_UTILS_GAMEPAD_INFO_H
#define SOSAGE_UTILS_GAMEPAD_INFO_H

#include <Sosage/Utils/enum.h>

#include <string>

namespace Sosage
{

enum Gamepad_labels { NO_LABEL, NINTENDO, XBOX };


struct Gamepad_info
{
  std::string id;
  std::string name;
  Gamepad_labels labels;
  bool ok_down;

  Gamepad_info (unsigned short vendor = 0, unsigned short product = 0,
                const std::string& name = "Default controller");
  Gamepad_info (const std::string& id, const Gamepad_labels& labels, const bool& ok_down);
};

std::string gamepad_label (const Gamepad_info& info, const Event_value& value);

}

#endif // SOSAGE_UTILS_GAMEPAD_INFO_H
