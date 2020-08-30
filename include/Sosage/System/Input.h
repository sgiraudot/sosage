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
#include <Sosage/System/Handle.h>

namespace Sosage::System
{

class Input : public Base
{
  Core::Input m_core;

  struct Virtual_cursor
  {
    bool down = false;
    bool has_moved = false;
    Time::Unit time;
    int x;
    int y;
    double cursor_x;
    double cursor_y;
  };

  bool m_alt;
  Virtual_cursor m_virtual_cursor;

public:

  Input (Content& content);

  virtual void run ();

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INPUT_H
