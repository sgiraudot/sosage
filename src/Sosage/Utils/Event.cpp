/*
  [src/Sosage/Utils/Event.cpp]
  Definitions of all possible events.

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

#include <Sosage/Utils/Event.h>

#include <sstream>

namespace Sosage
{

std::ostream& operator<< (std::ostream& os, const Event_type& type)
{
  if (type == EMPTY) os << "EMPTY";
  else if (type == UNUSED) os << "UNUSED";
  else if (type == WINDOW) os << "WINDOW";
  else if (type == MOUSE_DOWN) os << "MOUSE_DOWN";
  else if (type == MOUSE_MOVE) os << "MOUSE_MOVE";
  else if (type == MOUSE_UP) os << "MOUSE_UP";
  else if (type == TOUCH_DOWN) os << "TOUCH_DOWN";
  else if (type == TOUCH_MOVE) os << "TOUCH_MOVE";
  else if (type == TOUCH_UP) os << "TOUCH_UP";
  else if (type == KEY_DOWN) os << "KEY_DOWN";
  else if (type == KEY_UP) os << "KEY_UP";
  else if (type == NEW_GAMEPAD) os << "NEW_GAMEPAD";
  else if (type == BUTTON_DOWN) os << "BUTTON_DOWN";
  else if (type == BUTTON_UP) os << "BUTTON_UP";
  else if (type == STICK_MOVE) os << "STICK_MOVE";
  else os << "WTF";
  return os;
}


std::ostream& operator<< (std::ostream& os, const Event_value& value)
{
  if (value == NONE) os << "NONE";
  else if (value == EXIT) os << "EXIT";
  else if (value == RESIZED) os << "RESIZED";
  else if (value == LEFT) os << "LEFT";
  else if (value == RIGHT) os << "RIGHT";
  else if (value == ALT) os << "ALT";
  else if (value == CTRL) os << "CTRL";
  else if (value == SHIFT) os << "SHIFT";
  else if (value == TAB) os << "TAB";
  else if (value == ANDROID_BACK) os << "ANDROID_BACK";
  else if (value == ENTER) os << "ENTER";
  else if (value == ESCAPE) os << "ESCAPE";
  else if (value == SPACE) os << "SPACE";
  else if (value == UP_ARROW) os << "UP_ARROW";
  else if (value == LEFT_ARROW) os << "LEFT_ARROW";
  else if (value == RIGHT_ARROW) os << "RIGHT_ARROW";
  else if (value == DOWN_ARROW) os << "DOWN_ARROW";
  else if (value == NORTH) os << "NORTH";
  else if (value == EAST) os << "EAST";
  else if (value == WEST) os << "WEST";
  else if (value == SOUTH) os << "SOUTH";
  else if (value == LEFT_SHOULDER) os << "LEFT_SHOULDER";
  else if (value == RIGHT_SHOULDER) os << "RIGHT_SHOULDER";
  else if (value == START) os << "START";
  else if (value == SELECT) os << "SELECT";
  else if (A <= value && value <= Z) os << char('A' + (value - A));
  else if (F1 <= value && value <= F12) os << "F" << 1 + value - F1;
  else os << "WTF";
  return os;
}

Event::Event (const Type& type, const Value& value, const int& x, const int& y)
  : m_type (type), m_value (value), m_x (x), m_y (y)
{ }

const Event::Type& Event::type() const
{
  return m_type;
}

const Event::Value& Event::value() const
{
  return m_value;
}

const int& Event::x() const
{
  return m_x;
}

const int& Event::y() const
{
  return m_y;
}

bool Event::operator< (const Event& other) const
{
  if (m_type == other.m_type)
    return m_value < other.m_value;
  return m_type < other.m_type;
}

bool Event::operator== (const Event& other) const
{
  return (m_type == other.m_type &&
          m_value == other.m_value);
}

Event::operator bool() const
{
  return (m_type != EMPTY);
}

std::string Event::to_string() const
{
  std::ostringstream oss;
  oss << *this;
  return oss.str();
}

} // namespace Sosage
