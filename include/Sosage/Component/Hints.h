/*
  [include/Sosage/Component/Hints.h]
  System for storing and selecting hints.

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

#ifndef SOSAGE_COMPONENT_HINTS_H
#define SOSAGE_COMPONENT_HINTS_H

#include <Sosage/Component/Conditional.h>
#include <Sosage/Component/Handle.h>

#include <vector>

namespace Sosage::Component
{

class Hints : public Base
{
private:

  std::vector<Conditional_base_handle> m_values;
  
public:

  Hints (const std::string& id);
  std::string next() const;
  
  void add (Conditional_base_handle condition) { m_values.push_back (condition); }
};

typedef std::shared_ptr<Hints> Hints_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HINT_H
