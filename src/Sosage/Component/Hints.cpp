/*
  [src/Sosage/Component/Hints.cpp]
  System for storing and selecting hints.

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
#include <Sosage/Component/Hints.h>

namespace Sosage::Component
{

Hints::Hints (const std::string& id)
  : Base (id)
{ }

std::string Hints::next() const
{
  // Return first hint that hits a fullfilled condition
  for (const Conditional_base_handle& condition : m_values)
    if (auto str = cast<Component::String>(condition))
      return str->value();
  return "";
}


} // namespace Sosage::Component
