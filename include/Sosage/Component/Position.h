/*
  [include/Sosage/Component/Position.h]
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

#ifndef SOSAGE_COMPONENT_POSITION_H
#define SOSAGE_COMPONENT_POSITION_H

#include <Sosage/Component/Image.h>
#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/geometry.h>

#include <vector>

namespace Sosage::Component
{

class Position : public Base
{
public:

  Position (const std::string& id) : Base(id) { }

  virtual void set (const Point& p) = 0;
  virtual Point value() const = 0;
  virtual bool absolute() const = 0;
};

using Position_handle = std::shared_ptr<Position>;

class Absolute_position : public Position
{
  Point m_pos;
  bool m_absolute;

public:

  Absolute_position (const std::string& id, const Point& coord, bool absolute = true);
  virtual std::string str() const;
  virtual Point value () const { return m_pos; }
  virtual void set (const Point& p) { m_pos = p; }
  bool absolute() const { return m_absolute; }
  bool& absolute() { return m_absolute; }
};

using Absolute_position_handle = std::shared_ptr<Absolute_position>;


class Relative_position : public Position
{
  Position_handle m_ref;
  Sosage::Vector m_diff;
  double m_factor;

public:

  Relative_position (const std::string& id, Position_handle ref,
                     const Sosage::Vector& diff = Sosage::Vector(0,0),
                     double factor = 1.);

  Absolute_position_handle absolute_reference()
  {
    if (auto r = cast<Absolute_position>(m_ref))
      return r;
    return cast<Relative_position>(m_ref)->absolute_reference();
  }

  virtual Point value() const { return m_factor * m_ref->value() + m_diff; }
  virtual void set (const Point& p) { m_diff = Sosage::Vector(m_factor * m_ref->value(), p); }
  void set (const Sosage::Vector& v) { m_diff = v; }
  bool absolute() const { return m_ref->absolute(); }
};

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_POSITION_H
