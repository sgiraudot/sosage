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

#include <Sosage/Component/Base.h>
#include <Sosage/Utils/enum.h>

#include <stack>

namespace Sosage::Component
{

class Status : public Value<Sosage::Status>
{
private:

  using Base = Value<Sosage::Status>;
  mutable std::stack<Sosage::Status> m_value;
  
public:

  Status (const std::string& entity, const std::string& component);
  void push (const Sosage::Status& v);
  void pop();
  virtual Sosage::Status value() const;
  Sosage::Status next_value() const;
  bool is (const Sosage::Status& s);
  bool is (const Sosage::Status& s1, const Sosage::Status& s2);
  bool is (const Sosage::Status& s1, const Sosage::Status& s2, const Sosage::Status& s3);
  bool was (const Sosage::Status& s);
  virtual std::string str() const;
};

using Status_handle = std::shared_ptr<Status>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_STATUS_H
