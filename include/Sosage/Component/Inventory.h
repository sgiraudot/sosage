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

#include <Sosage/Component/Base.h>

#include <vector>

namespace Sosage
{

namespace Config
{
constexpr int displayed_inventory_size = 9;
} // namespace Config

namespace Component
{

class Inventory : public Base
{
  std::vector<std::string> m_data;
  std::size_t m_position;
  
public:

  Inventory (const std::string& entity, const std::string& component);
  void clear();
  std::vector<std::string>::const_iterator begin() const;
  std::vector<std::string>::const_iterator end() const;
  void add (const std::string& entity);
  void remove (const std::string& entity);
  bool next();
  bool prev();
  std::size_t size() const;
  std::size_t position() const;
  std::string get (std::size_t i) const;
  const std::vector<std::string>& data() const;

  STR_NAME("Inventory");
};

using Inventory_handle = std::shared_ptr<Inventory>;

} // namespace Component

} // namespace Sosage

#endif // SOSAGE_COMPONENT_INVENTORY_H
