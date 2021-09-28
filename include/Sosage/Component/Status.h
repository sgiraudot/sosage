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
#include <Sosage/Utils/enum.h>

#include <stack>

namespace Sosage
{

namespace Component
{

class Status : public Value<Sosage::Status>
{
private:

  using Base = Value<Sosage::Status>;
  mutable std::stack<Sosage::Status> m_value;
  
public:

  Status (const std::string& id) : Base(id) { m_value.push(IDLE); }
  void push (const Sosage::Status& v) { m_value.push(v); }
  void pop () { m_value.pop(); }
  virtual Sosage::Status value() const { return m_value.top(); }
  Sosage::Status next_value() const
  {
    if (m_value.size() == 1)
      return value();
    Sosage::Status saved = m_value.top();
    m_value.pop();
    Sosage::Status out = m_value.top();
    m_value.push(saved);
    return out;
  }

  bool is (const Sosage::Status& s) { return (value() == s); }
  bool is (const Sosage::Status& s1, const Sosage::Status& s2) { return (value() == s1) || (value() == s2); }
  bool is (const Sosage::Status& s1, const Sosage::Status& s2, const Sosage::Status& s3)
  { return (value() == s1) || (value() == s2) || (value() == s3); }
  bool was (const Sosage::Status& s) { return (next_value() == s); }

  virtual std::string str() const
  {
    auto copy = m_value;
    std::string out = "STATUS:";
    while (!copy.empty())
    {
      Sosage::Status s = copy.top();
      copy.pop();
      if (s == IDLE) out += " idle";
      else if (s == CUTSCENE) out += " cutscene";
      else if (s == PAUSED) out += " pause";
      else if (s == LOCKED) out += " locked";
      else if (s == ACTION_CHOICE) out += " action";
      else if (s == INVENTORY_ACTION_CHOICE) out += " inventory_action";
      else if (s == DIALOG_CHOICE) out += " dialog";
      else if (s == OBJECT_CHOICE) out += " object";
      else if (s == IN_INVENTORY) out += " in_inventory";
      else if (s == IN_MENU) out += " in_menu";
      else if (s == IN_WINDOW) out += " in_window";
      else if (s == IN_CODE) out += " in_code";
    }
    return out;
  }

};

using Status_handle = std::shared_ptr<Status>;

} // namespace Component

} // namespace Sosage

#endif // SOSAGE_COMPONENT_STATUS_H
