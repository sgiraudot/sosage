/*
  [src/Sosage/Component/Conditional.cpp]
  Access different components depending on a condition.

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

#include <Sosage/Component/Conditional.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/error.h>

namespace Sosage::Component
{

Conditional_base::Conditional_base (const std::string& entity, const std::string& component)
  : Base(entity, component)
{ }

Conditional::Conditional (const std::string& entity, const std::string& component,
                          Condition_handle condition,
                          Handle if_true,
                          Handle if_false)
  : Conditional_base(entity, component)
  , m_condition (condition)
  , m_if_true (if_true)
  , m_if_false (if_false)
{ }

Conditional::~Conditional()
{
  m_condition = Condition_handle();
  m_if_true = Handle();
  m_if_false = Handle();
}

Handle Conditional::get() const
{
  return (m_condition->value() ? m_if_true : m_if_false);
}

Random_conditional::Random_conditional (const std::string& entity, const std::string& component)
  : Conditional_base(entity, component)
{ }

Random_conditional::~Random_conditional()
{
  m_handles.clear();
}

void Random_conditional::add (Handle h)
{
  m_handles.emplace_back(h);
}

Handle Random_conditional::get() const
{
  return random_choice(m_handles);
}


} // namespace Sosage::Component
