/*
  [src/Sosage/Component/Position.cpp]
  2D coordinates.

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

#include <Sosage/Component/cast.h>
#include <Sosage/Component/Position.h>

namespace Sosage::Component
{

Position::Position (const std::string& id)
  : Base(id)
{ }

Absolute_position::Absolute_position (const std::string& id, const Point& point, bool absolute)
  : Position(id), m_pos (point), m_absolute (absolute)
{ }

std::string Absolute_position::str() const
{
  return this->id() + " (" + std::to_string (m_pos.x())
    + ";" + std::to_string(m_pos.y()) + ")";
}

Point Absolute_position::value () const
{
  return m_pos;
}

void Absolute_position::set (const Point& p)
{
  m_pos = p;
}

bool Absolute_position::absolute() const
{
  return m_absolute;
}

bool& Absolute_position::absolute()
{
  return m_absolute;
}

Relative_position::Relative_position (const std::string& id, Position_handle ref,
                                      const Sosage::Vector& diff, double factor)
  : Position(id), m_ref(ref), m_diff(diff), m_factor(factor)
{ }
  
Absolute_position_handle Relative_position::absolute_reference()
{
  if (auto r = cast<Absolute_position>(m_ref))
    return r;
  return cast<Relative_position>(m_ref)->absolute_reference();
}

Point Relative_position::value() const
{
  return m_factor * m_ref->value() + m_diff;
}

void Relative_position::set (const Point& p)
{
  m_diff = Sosage::Vector(m_factor * m_ref->value(), p);
}

void Relative_position::set (const Sosage::Vector& v)
{
  m_diff = v;
}

bool Relative_position::absolute() const
{
  return m_ref->absolute();
}

} // namespace Sosage::Component
