/*
  [include/Sosage/Component/Group.h]
  Group of components with factorized modifications.

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

#ifndef SOSAGE_COMPONENT_GROUP_H
#define SOSAGE_COMPONENT_GROUP_H

#include <Sosage/Component/Base.h>
#include <Sosage/Component/cast.h>

#include <functional>
#include <vector>

namespace Sosage::Component
{

class Group : public Base
{
private:

  std::vector<Handle> m_items;

public:

  Group (const std::string& entity, const std::string& component);
  void add (Handle h);
  void remove (Handle h);

  template <typename Comp>
  void apply (const std::function<void(std::shared_ptr<Comp>)>& functor)
  {
    for (Handle h : m_items)
      if (auto c = cast<Comp>(h))
        functor (c);
      else if (auto g = cast<Group>(h)) // Recursively call on nested groups
        g->apply<Comp> (functor);
  }

  STR_NAME("Group");
  STR_SUB(std::string out = "";
          for (Handle h : m_items)
            out += component_str(h, indent+1, "Item = ");
          return out;);
};

using Group_handle = std::shared_ptr<Group>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GROUP_H
