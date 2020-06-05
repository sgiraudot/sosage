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

#include <Sosage/Component/Position.h>

namespace Sosage::Component
{

Position::Position (const std::string& id, const Point& point, bool absolute)
  : Base(id), m_pos (point), m_absolute (absolute)
{ }

std::string Position::str() const
{
  return this->id() + " (" + std::to_string (m_pos.x())
    + ";" + std::to_string(m_pos.y()) + ")";
}
  
} // namespace Sosage::Component
