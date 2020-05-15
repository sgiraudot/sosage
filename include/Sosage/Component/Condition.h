/*
  [include/Sosage/Component/Condition.h]
  Boolean logic conditions.

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

#ifndef SOSAGE_COMPONENT_CONDITION_H
#define SOSAGE_COMPONENT_CONDITION_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Component/Simple.h>

namespace Sosage::Component
{

class Condition : public Base
{
public:

  Condition (const std::string& id);
  virtual bool value() const = 0;
  virtual std::string str() const;
};

typedef std::shared_ptr<Condition> Condition_handle;

class Boolean : public Condition
{
  bool m_value;
public:

  Boolean (const std::string& id, const bool& value)
    : Condition(id), m_value(value) { }
  void set(const bool& value) { m_value = value; }
  void toggle() { m_value = !m_value; }
  virtual bool value() const { return m_value; }
};

typedef std::shared_ptr<Boolean> Boolean_handle;

class State_boolean : public Condition
{
  String_handle m_state;
  std::string m_value;
  
public:

  State_boolean (const std::string& id, String_handle state, const std::string& value)
    : Condition(id), m_state (state), m_value(value) { }

  virtual bool value() const { return (m_state->value() == m_value); }
};

typedef std::shared_ptr<State_boolean> State_boolean_handle;

class And : public Condition
{
  std::pair<Condition_handle, Condition_handle> m_values;
  
public:

  And (const std::string& id,
       Condition_handle first, Condition_handle second)
    : Condition(id), m_values(first, second) { }
  virtual bool value() const { return (m_values.first->value() && m_values.second->value()); }
};

typedef std::shared_ptr<And> And_handle;

class Or : public Condition
{
  std::pair<Condition_handle, Condition_handle> m_values;
  
public:

  Or (const std::string& id,
      Condition_handle first, Condition_handle second)
    : Condition(id), m_values(first, second) { }
  virtual bool value() const { return (m_values.first->value() || m_values.second->value()); }
};

typedef std::shared_ptr<Or> Or_handle;


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITION_H
