/*
  [src/Sosage/Component/Base.cpp]
  Virtual basis for all components.

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

#include <Sosage/Component/Base.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage::Component
{

Base::Base(const std::string& id) : m_id (id)
{
  std::size_t pos = m_id.find_first_of(':');
  if (pos == std::string::npos)
    return;
  // This is a duplicate storage, but as we both need to often use id and entity/component,
  // it's better to lose a bit of memory and avoid always creating strings
  m_entity = std::string (m_id.begin(), m_id.begin() + pos);
  m_component = std::string (m_id.begin() + pos + 1, m_id.end());
}

Base::~Base() { }

bool Base::is_system() const
{
  return isupper(m_id[0]);
}

const std::string& Base::id() const
{
  return m_id;
}

const std::string& Base::entity() const
{
  return m_entity;
}

// Special handling of entity for characters
std::string Base::character_entity() const
{
  for (const std::string& postfix : { "_body", "_head", "_mouth" })
  {
    std::size_t pos = m_id.find(postfix);
    if (pos != std::string::npos)
      return std::string (m_id.begin(), m_id.begin() + pos);
  }
  return entity();
}

// Special handling of entity for binary actions
std::string Base::target_entity() const
{
  for (const std::string& prefix : { "_inventory_" })
  {
    std::size_t pos = m_id.find(prefix);
    if (pos != std::string::npos)
      return std::string (m_id.begin(), m_id.begin() + pos);
  }
  return std::string (m_id.begin(), m_id.begin() + m_id.find_last_of('_'));
}

const std::string& Base::component() const
{
  SOSAGE_COUNT(Component__component);
  return m_component;
}

std::string Base::str() const
{
  return m_id;
}

} // namespace Sosage::Component
