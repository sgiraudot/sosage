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
  Point m_pos;
  bool m_absolute;

public:

  Position (const std::string& id, const Point& coord, bool absolute = true);
  virtual std::string str() const;
  Point value () const { return m_pos; }
  void set (const Point& p) { m_pos = p; }
  bool absolute() const { return m_absolute; }
};
typedef std::shared_ptr<Position> Position_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_POSITION_H
