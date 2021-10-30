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

namespace Sosage
{

std::string gamepad_label (const Gamepad_type& type, const Event_value& value);

}

#endif // SOSAGE_UTILS_GAMEPAD_LABELS_H
