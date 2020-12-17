/*
  [include/Sosage/Utils/Event.h]
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

#ifndef SOSAGE_UTILS_EVENT_H
#define SOSAGE_UTILS_EVENT_H

#include <Sosage/Utils/enum.h>
#include <Sosage/Utils/error.h>

#include <iostream>
#include <sstream>

namespace Sosage
{

inline std::ostream& operator<< (std::ostream& os, const Event_type& type)
{
  if (type == EMPTY) os << "EMPTY";
  else if (type == WINDOW) os << "WINDOW";
  else if (type == CURSOR_DOWN) os << "CURSOR_DOWN";
  else if (type == CURSOR_MOVE) os << "CURSOR_MOVE";
  else if (type == CURSOR_UP) os << "CURSOR_UP";
  else if (type == KEY_DOWN) os << "KEY_DOWN";
  else if (type == KEY_UP) os << "KEY_UP";
  else os << "WTF";
  return os;
}


inline std::ostream& operator<< (std::ostream& os, const Event_value& value)
{
  if (value == NONE) os << "NONE";
  else if (value == EXIT) os << "EXIT";
  else if (value == RESIZED) os << "RESIZED";
  else if (value == LEFT) os << "LEFT";
  else if (value == RIGHT) os << "RIGHT";
  else if (value == ALT) os << "ALT";
  else if (value == ANDROID_BACK) os << "ANDROID_BACK";
  else if (value == ENTER) os << "ENTER";
  else if (value == ESCAPE) os << "ESCAPE";
  else if (value == SPACE) os << "SPACE";
  else if (A <= value && value <= Z) os << char('A' + (value - A));
  else if (F1 <= value && value <= F12) os << "F" << 1 + value - F1;
  else os << "WTF";
  return os;
}

class Event
{
public:

  using Type = Event_type;
  using Value = Event_value;

private:

  Type m_type;
  Value m_value;
  int m_x;
  int m_y;

public:

  Event (const Type& type = EMPTY,
         const Value& value = NONE,
         const int& x = -1,
         const int& y = -1)
    : m_type (type), m_value (value), m_x (x), m_y (y)
  { }

  const Type& type() const { return m_type; }
  const Value& value() const { return m_value; }
  const int& x() const { return m_x; }
  const int& y() const { return m_y; }

  bool operator< (const Event& other) const
  {
    if (m_type == other.m_type)
      return m_value < other.m_value;
    return m_type < other.m_type;
  }

  bool operator== (const Event& other) const
  {
    return (m_type == other.m_type &&
            m_value == other.m_value);
  }

  operator bool() const
  {
    return (m_type != EMPTY);
  }

  std::string to_string() const
  {
    std::ostringstream oss;
    oss << *this;
    return oss.str();
  }

  friend std::ostream& operator<< (std::ostream& os, const Event& ev)
  {
    if (ev == Event())
    {
      os << "Event(null)";
      return os;
    }
    os << "Event(" << ev.type() << ", " << ev.value()
       << ", " << ev.x() << ", " << ev.y() << ")";
    return os;
  }

};


}

#endif // SOSAGE_UTILS_EVENT_H
