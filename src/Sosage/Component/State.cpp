/*
  [src/Sosage/Component/State.cpp]
  Handles different states of an entity.

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

#include <Sosage/Component/State.h>

namespace Sosage::Component
{

State::State (const std::string& id, const std::string& value)
  : Base(id), m_value (value)
{ }

State::~State()
{ }

std::string State::str() const
{
  return this->id() + " " + m_value;
}

} // namespace Sosage::Component
