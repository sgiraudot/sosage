/*
  [src/Sosage/Component/Status.cpp]
  The game current status (and pending ones).

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

#include <Sosage/Component/Status.h>

namespace Sosage::Component
{

Status::Status (const std::string& entity, const std::string& component)
  : Base(entity, component)
{
  reset();
}

void Status::reset()
{
  while (!m_value.empty())
    m_value.pop();
  m_value.push(IDLE);
  m_value.push(LOCKED); // start game locked
}

void Status::push (const Sosage::Status& v)
{
  m_value.push(v);
}

void Status::pop ()
{
  m_value.pop();
}

Sosage::Status Status::value() const
{
 return m_value.top();
}

Sosage::Status Status::next_value() const
{
  if (m_value.size() == 1)
    return value();
  Sosage::Status saved = m_value.top();
  m_value.pop();
  Sosage::Status out = m_value.top();
  m_value.push(saved);
  return out;
}

bool Status::is (const Sosage::Status& s)
{
  return (value() == s);
}

bool Status::is (const Sosage::Status& s1, const Sosage::Status& s2)
{
  return (value() == s1) || (value() == s2);
}

bool Status::is (const Sosage::Status& s1, const Sosage::Status& s2, const Sosage::Status& s3)
{
  return (value() == s1) || (value() == s2) || (value() == s3);
}

bool Status::was (const Sosage::Status& s)
{
  return (next_value() == s);
}

} // namespace Sosage::Component
