/*
  [src/Sosage/Component/Path.cpp]
  List of coordinates that the character should follow.

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

#include <Sosage/Component/Path.h>

namespace Sosage::Component
{

Path::Path (const std::string& id, const Point& point)
  : Base(id), m_steps (1, point), m_current(0)
{ }

Path::Path (const std::string& id, std::vector<Point>& steps)
  : Base(id), m_current(0)
{
  m_steps.swap(steps);
}

std::string Path::str() const
{
  std::string out = this->id();
  for (const Point& p : m_steps)
    out += " (" + std::to_string(p.x()) + ";" + std::to_string(p.y()) + ")";
  return out;
}

} // namespace Sosage::Component
