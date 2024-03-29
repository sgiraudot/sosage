/*
  [src/Sosage/Component/Condition.cpp]
  Boolean logic conditions.

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

#include <Sosage/Component/Condition.h>

namespace Sosage::Component
{

Condition::Condition (const std::string& entity, const std::string& component)
  : Value(entity, component)
{ }

std::string Condition::str() const
{
  return Base::str() + " " + (value() ? "TRUE" : "FALSE");
}

Boolean::Boolean (const std::string& entity, const std::string& component, const bool& value)
  : Condition(entity, component), m_value(value), m_memory(value)
{ }

void Boolean::set (const bool& value)
{
  m_value = value;
  mark_as_altered();
}

void Boolean::toggle()
{
  m_value = !m_value;
}

bool Boolean::value() const
{
  return m_value;
}

void Boolean::begin_temporary_true()
{
  m_memory = m_value;
  m_value = true;
}

void Boolean::end_temporary_true()
{
  m_value = m_memory;
}

bool Functional_condition::value() const
{
  return m_function (m_arg);
}

And::And (const std::string& entity, const std::string& component,
          Condition_handle first, Condition_handle second)
  : Condition(entity, component), m_values(first, second)
{ }

bool And::value() const
{
  return (m_values.first->value() && m_values.second->value());
}

Or::Or (const std::string& entity, const std::string& component,
        Condition_handle first, Condition_handle second)
  : Condition(entity, component), m_values(first, second)
{ }

bool Or::value() const
{
  return (m_values.first->value() || m_values.second->value());
}

Not::Not (const std::string& entity, const std::string& component, Condition_handle value)
  : Condition(entity, component), m_value(value)
{ }

bool Not::value() const
{
  return !(m_value->value());
}

And_handle make_and (Condition_handle h0, Condition_handle h1)
{
  return make_handle<And>(h0->entity() + "_" + h1->entity(), "and", h0, h1);
}

Or_handle make_or (Condition_handle h0, Condition_handle h1)
{
  return make_handle<Or>(h0->entity() + "_" + h1->entity(), "or", h0, h1);
}

Not_handle make_not (Condition_handle handle)
{
  return make_handle<Not>(handle->entity(), "not", handle);
}

} // namespace Sosage::Component
