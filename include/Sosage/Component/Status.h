/*
  [include/Sosage/Component/Status.h]
  The game current status (and pending ones).

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

#ifndef SOSAGE_COMPONENT_STATUS_H
#define SOSAGE_COMPONENT_STATUS_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Content.h>
#include <Sosage/Utils/time.h>

#include <stack>

namespace Sosage
{

enum Status { IDLE, LOADING, PAUSED, LOCKED, IN_MENU, IN_WINDOW, IN_CODE };

namespace Component
{

class Status : public Value<Sosage::Status>
{
private:

  typedef Value<Sosage::Status> Base;
  std::stack<Sosage::Status> m_value;
  
public:

  Status (const std::string& id) : Base(id) { m_value.push(IDLE); }
  void push (const Sosage::Status& v) { m_value.push(v); }
  void pop () { m_value.pop(); }
  virtual Sosage::Status value() const { return m_value.top(); }
};

typedef std::shared_ptr<Status> Status_handle;

} // namespace Component

} // namespace Sosage

#endif // SOSAGE_COMPONENT_STATUS_H