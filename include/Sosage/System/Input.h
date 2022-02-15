/*
  [include/Sosage/System/Input.h]
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
#include <Sosage/System/Base.h>

namespace Sosage
{

namespace Config
{
constexpr double stick_max = 32767.5;
} // namespace Config

namespace System
{

class Input : public Base
{
  Core::Input m_core;
  std::vector<Event> m_current_events;
  std::vector<bool> m_keys_on;
  double m_x;
  double m_y;
  bool m_fake_touchscreen;

public:

  Input (Content& content);

  virtual void run ();

private:

  typename std::vector<bool>::reference key_on(const Event_value& value);
};

} // namespace System

} // namespace Sosage

#endif // SOSAGE_SYSTEM_INPUT_H
