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

#include <iostream>

namespace Sosage
{

std::ostream& operator<< (std::ostream& os, const Event_type& type);
std::ostream& operator<< (std::ostream& os, const Event_value& value);

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
         const int& y = -1);
  const Type& type() const;
  const Value& value() const;
  const int& x() const;
  const int& y() const;
  bool operator< (const Event& other) const;
  bool operator== (const Event& other) const;
  operator bool() const;
  std::string to_string() const;
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

} // namespace Sosage

#endif // SOSAGE_UTILS_EVENT_H
