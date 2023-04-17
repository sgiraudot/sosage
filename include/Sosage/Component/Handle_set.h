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

struct Id_hash
{
  std::size_t operator() (const Id& id) const
  {
    return std::hash<std::string>()(id.first) ^ std::hash<std::string>()(id.second);
  }
};

using Handle_map = std::unordered_map<Id, Handle, Id_hash>;
using Component_map = std::vector<Handle_map>;

class Handle_set
{
  Handle_map& m_map;

public:

  class iterator
  {
    using Base = Handle_map::iterator;
    Base m_base;

  public:

    using iterator_category = std::forward_iterator_tag;
    using difference_type   = std::ptrdiff_t;
    using value_type        = Handle;
    using pointer           = value_type*;
    using reference         = value_type&;

    iterator (Base base = Base()) : m_base(base) { }

    reference operator*() const { return m_base->second; }
    pointer operator->() { return &m_base->second; }
    iterator& operator++() { m_base++; return *this; }
    iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
    friend bool operator== (const iterator& a, const iterator& b) { return a.m_base == b.m_base; };
    friend bool operator!= (const iterator& a, const iterator& b) { return a.m_base != b.m_base; };
  };

  using const_iterator = iterator;

  Handle_set (Handle_map& map) : m_map (map) { }
  void clear() { m_map.clear(); }
  iterator begin() { return iterator(m_map.begin()); }
  iterator end() { return iterator(m_map.end()); }
  const_iterator begin() const { return iterator(m_map.begin()); }
  const_iterator end() const { return iterator(m_map.end()); }
};

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_SET_H
