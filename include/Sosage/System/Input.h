/*
  [include/Sosage/System/Graphic.h]
  Reads and interprets user input (keyboard, mouse, touchscreen).

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

#ifndef SOSAGE_SYSTEM_INPUT_H
#define SOSAGE_SYSTEM_INPUT_H

#include <Sosage/Content.h>
#include <Sosage/Core/Input.h>

namespace Sosage::System
{

class Input
{
  Content& m_content;
  Core::Input m_core;

  bool m_alt;

public:

  Input (Content& content);

  void run ();
  
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INPUT_H
