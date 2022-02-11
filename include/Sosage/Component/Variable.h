/*
  [include/Sosage/Component/Variable.h]
  Handle to point variably to another handle.

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

#ifndef SOSAGE_COMPONENT_VARIABLE_H
#define SOSAGE_COMPONENT_VARIABLE_H

#include <Sosage/Component/Conditional.h>

namespace Sosage::Component
{

class Variable : public Conditional_base
{
  Handle m_target;

public:

  Variable (const std::string& entity, const std::string& component, Handle target);
  void set (Handle target);
  virtual Handle get() const;
};

using Variable_handle = std::shared_ptr<Variable>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_VARIABLE_H
