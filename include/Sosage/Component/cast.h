/*
  [include/Sosage/Component/cast.h]
  Cast operator to easily convert component handles.

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

#ifndef SOSAGE_COMPONENT_CAST_H
#define SOSAGE_COMPONENT_CAST_H

#include <Sosage/Component/Conditional.h>

#include <memory>

namespace Sosage::Component
{

template <typename T>
inline std::shared_ptr<T> cast (Handle h)
{
  std::shared_ptr<T> out = std::dynamic_pointer_cast<T>(h);
  if (out)
    return out;

  Conditional_base_handle cond = std::dynamic_pointer_cast<Conditional_base>(h);
  if (cond)
    return cast<T>(cond->get());

  return std::shared_ptr<T>();
}


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CAST_H
