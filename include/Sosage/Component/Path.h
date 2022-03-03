/*
  [include/Sosage/Component/Path.h]
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

#ifndef SOSAGE_COMPONENT_PATH_H
#define SOSAGE_COMPONENT_PATH_H

#include <Sosage/Component/Base.h>
#include <Sosage/Utils/geometry.h>

#include <vector>

namespace Sosage::Component
{

class Path : public Base
{
  std::vector<Point> m_steps;
  std::size_t m_current;

public:

  Path (const std::string& entity, const std::string& component, const Point& coord);
  Path (const std::string& entity, const std::string& component, std::vector<Point>& steps);
  std::size_t size() const;
  const Point& operator[] (const std::size_t& idx) const;
  Point& operator[] (const std::size_t& idx);
  const std::size_t& current() const;
  std::size_t& current();

  STR_NAME("Path");
};

using Path_handle = std::shared_ptr<Path>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_PATH_H
