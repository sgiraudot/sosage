/*
  [include/Sosage/Component/State.h]
  Handles different states of an entity.

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

#ifndef SOSAGE_COMPONENT_STATE_H
#define SOSAGE_COMPONENT_STATE_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class State : public Base
{
  std::string m_value;
  
public:

  State (const std::string& id, const std::string& value = std::string());
  virtual ~State();
  virtual std::string str() const;
  const std::string& value() const { return m_value; }
  void set (const std::string& value) { m_value = value; }
};

typedef std::shared_ptr<State> State_handle;

} // namespace Sosage::State

#endif // SOSAGE_COMPONENT_STATE_H
