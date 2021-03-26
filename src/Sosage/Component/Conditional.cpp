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

namespace Sosage::Component
{

Conditional::Conditional (const std::string& id,
                          Condition_handle condition,
                          Handle if_true,
                          Handle if_false)
  : Conditional_base(id)
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

std::string Conditional::str() const
{
  std::string ift = "[" + (m_if_true ? m_if_true->str() : "NULL") + "]";
  std::string iff = "[" + (m_if_false ? m_if_false->str() : "NULL") + "]";
  return this->id() + " -> " + m_condition->id() + " ? "
    + ift + " : " + iff;
}

Handle Conditional::get() const
{
  return (m_condition->value() ? m_if_true : m_if_false);
}

String_conditional::String_conditional (const std::string& id,
                                        String_handle state)
  : Conditional_base(id)
  , m_state (state)
{ }

String_conditional::~String_conditional()
{
  m_state = String_handle();
  m_handles.clear();
}

std::string String_conditional::str() const
{
  return this->id() + " -> " + m_state->id() + " ? "
    + (get() ? get()->id() : "NULL");
}

void String_conditional::add (const std::string& state, Handle h)
{
  m_handles.insert (std::make_pair (state, h));
}
  
void String_conditional::set (const std::string& state, Handle h)
{
  auto iter = m_handles.find(state);
  dbg_check(iter != m_handles.end(), "State " + state + " not found in string conditional " + id());
  iter->second = h;
}

Handle String_conditional::get() const
{
  auto iter
    = m_handles.find(m_state->value());
  if (iter == m_handles.end())
    return Handle();
//  check (iter != m_handles.end(), "Cannot find state " + m_state->value() + " in " + id());
  return iter->second;
}

Random_conditional::Random_conditional (const std::string& id)
  : Conditional_base(id)
  , m_total(0)
{ }

Random_conditional::~Random_conditional()
{
  m_handles.clear();
}

std::string Random_conditional::str() const
{
  return this->id() + " -> " + get()->id();
}

void Random_conditional::add (double probability, Handle h)
{
  m_handles.push_back (std::make_pair (probability, h));
  m_total += probability;
}
  
Handle Random_conditional::get() const
{
  double random = m_total * (rand() / double(RAND_MAX));
  double accu = 0;

  std::size_t idx = 0;
  while (random >= accu && idx < m_handles.size())
  {
    accu += m_handles[idx].first;
    ++ idx;
  }

  return m_handles[idx-1].second;
}


} // namespace Sosage::Component
