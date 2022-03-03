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
#include <Sosage/Utils/demangle.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage::Component
{

Base::Base (const std::string& entity, const std::string& component)
  : m_id (entity, component), m_altered(false)
{ }

Base::~Base() { }

bool Base::is_system() const
{
  return isupper(entity()[0]);
}

void Base::mark_as_altered()
{
  m_altered = true;
}

void Base::mark_as_unaltered()
{
  m_altered = false;
}

bool Base::was_altered() const
{
  return m_altered;
}

const Id& Base::id() const
{
  return m_id;
}

const std::string& Base::entity() const
{
  return m_id.first;
}

// Special handling of entity for characters
std::string Base::character_entity() const
{
  for (const std::string& postfix : { "_body", "_head", "_mouth" })
  {
    std::size_t pos = entity().find(postfix);
    if (pos != std::string::npos)
      return std::string (entity().begin(), entity().begin() + pos);
  }
  return entity();
}

// Special handling of entity for binary actions
std::string Base::target_entity() const
{
  for (const std::string& prefix : { "_inventory_" })
  {
    std::size_t pos = entity().find(prefix);
    if (pos != std::string::npos)
      return std::string (entity().begin(), entity().begin() + pos);
  }
  return std::string (entity().begin(), entity().begin() + entity().find_last_of('_'));
}

const std::string& Base::component() const
{
  SOSAGE_COUNT(Component__component);
  return m_id.second;
}

std::string Base::str() const
{
  std::string out = str_name() + "(" + m_id.first + ":" + m_id.second + ")";
  std::string v = str_value();
  if (v != "")
    out += " = " + v;
  return out;
}

std::string component_str (Handle handle, const std::size_t& indent, const std::string& prefix)
{
  return std::string(indent*2, '-') + prefix + (handle ? handle->str() : "nullptr") + "\n"
      + (handle ? handle->str_sub(indent) : "");
}

} // namespace Sosage::Component
