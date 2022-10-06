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

#include <Sosage/Component/Base.h>
#include <Sosage/Utils/geometry.h>
#include <Sosage/Utils/conversions.h>

#include <functional>

namespace Sosage::Component
{

class Position : public Base
{
public:

  using const_reference = Point;
  using value_type = Point;

  Position (const std::string& entity, const std::string& component);

  virtual void set (const Point& p) = 0;
  virtual Point value() const = 0;
  virtual bool is_interface() const = 0;
};

using Position_handle = std::shared_ptr<Position>;

class Absolute_position : public Position
{
  Point m_pos;
  bool m_is_interface;

public:

  Absolute_position (const std::string& entity, const std::string& component,
                     const Point& coord, bool is_interface = true);
  virtual Point value () const;
  virtual void set (const Point& p);
  bool is_interface() const;
  bool& is_interface();

  STR_NAME("Absolute_position");
  STR_VALUE("Point(" + to_string(m_pos.x()) + ";" + to_string(m_pos.y()) + ")");
};

using Absolute_position_handle = std::shared_ptr<Absolute_position>;


class Relative_position : public Position
{
  Position_handle m_ref;
  Sosage::Vector m_diff;
  double m_factor;

public:

  Relative_position (const std::string& entity, const std::string& component, Position_handle ref,
                     const Sosage::Vector& diff = Sosage::Vector(0,0),
                     double factor = 1.);
  Absolute_position_handle absolute_reference();
  virtual Point value() const;
  virtual void set (const Point& p);
  void set (const Sosage::Vector& v);
  bool is_interface() const;

  STR_NAME("Relative_position");
  STR_VALUE("Vector(" + to_string(m_diff.x()) + ";" + to_string(m_diff.y()) + ")");
  STR_SUB(return component_str(m_ref, indent+1, "Reference -> "););
};

using Relative_position_handle = std::shared_ptr<Relative_position>;

class Functional_position : public Position
{
  using Function = std::function<Point(const std::string&)>;
  Function m_function;
  std::string m_arg;
  bool m_is_interface;
  mutable Point m_tmp_point;

public:

  Functional_position (const std::string& entity, const std::string& component, const Function& function,
                       const std::string& arg,
                       bool is_interface = false);

  virtual Point value() const;
  const Function& function() const;
  virtual void set (const Point& p);
  void set (const Function& function);
  bool is_interface() const;

  STR_NAME("Functional_position");
  STR_VALUE("arg(" + m_arg + ")");
};

using Functional_position_handle = std::shared_ptr<Functional_position>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_POSITION_H
