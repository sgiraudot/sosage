/*
  [include/Sosage/Component/Handle_set.h]
  Set of component handles and component map.

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

#ifndef SOSAGE_COMPONENT_HANDLE_SET_H
#define SOSAGE_COMPONENT_HANDLE_SET_H

#include <Sosage/Component/Base.h>

#include <unordered_map>
#include <vector>

namespace Sosage::Component
{

struct Hash_id
{
  std::size_t operator() (const Handle& h) const
  { return std::hash<std::string>()(h->id()); }
};

struct Equal_ids
{
  bool operator() (const Handle& a, const Handle& b) const
  { return (a->id() == b->id()); }
};

using Handle_set = std::unordered_map<std::string, Handle>;
using Component_map = std::vector<Handle_set>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_SET_H
