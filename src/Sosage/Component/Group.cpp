/*
  [src/Sosage/Component/Group.cpp]
  Group of components with factorized modifications.

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

#include <Sosage/Component/Group.h>

namespace Sosage::Component
{

Group::Group (const std::string& id)
  : Base(id)
{ }

void Group::add (Handle h)
{
  m_items.emplace_back(h);
}

void Group::remove (Handle h)
{
  m_items.erase (std::find(m_items.begin(), m_items.end(), h));
}

} // namespace Sosage::Component
