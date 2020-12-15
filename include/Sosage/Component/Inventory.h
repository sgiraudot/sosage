/*
  [include/Sosage/Component/Inventory.h]
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

#ifndef SOSAGE_COMPONENT_INVENTORY_H
#define SOSAGE_COMPONENT_INVENTORY_H

#include <Sosage/Component/Handle.h>

#include <vector>

namespace Sosage::Component
{

class Inventory : public Base
{
  std::vector<std::string> m_data;
  std::size_t m_position;
  
public:

  Inventory (const std::string& id);

  std::vector<std::string>::const_iterator begin() const { return m_data.begin(); }
  std::vector<std::string>::const_iterator end() const { return m_data.end(); }

  void add (const std::string& entity);
  void remove (const std::string& entity);
  void next();
  void prev();

  std::size_t size() const;
  std::size_t position() const;
  std::string get (std::size_t i) const;
  const std::vector<std::string>& data() const { return m_data; }

};

typedef std::shared_ptr<Inventory> Inventory_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_INVENTORY_H
