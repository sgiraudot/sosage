/*
  [src/Sosage/Component/Inventory.cpp]
  List of items carried by the player.

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

#include <Sosage/Component/Inventory.h>
#include <Sosage/Config/config.h>

#include <iostream>

namespace Sosage::Component
{

Inventory::Inventory (const std::string& id)
  : Base (id), m_position(0)
{ }

void Inventory::clear()
{
  m_data.clear();
  m_position = 0;
}

std::vector<std::string>::const_iterator Inventory::begin() const
{
  return m_data.begin();
}

std::vector<std::string>::const_iterator Inventory::end() const
{
  return m_data.end();
}

void Inventory::add (const std::string& entity)
{
  m_data.push_back (entity);
}

void Inventory::remove (const std::string& entity)
{
  for (typename std::vector<std::string>::iterator
         it = m_data.begin(); it != m_data.end(); ++ it)
    if (*it == entity)
    {
      m_data.erase(it);
      break;
    }
}

void Inventory::next()
{
  if (m_position < m_data.size() - Config::displayed_inventory_size)
    ++ m_position;
}

void Inventory::prev()
{
  if (m_position != 0)
    -- m_position;
}

std::size_t Inventory::size() const
{
  return m_data.size();
}

std::size_t Inventory::position() const
{
  return m_position;
}
  

std::string Inventory::get (std::size_t i) const
{
  return m_data[i];
}

const std::vector<std::string>& Inventory::data() const
{
  return m_data;
}

} // namespace Sosage::Component
