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

namespace Sosage::Component
{

class Condition : public Value<bool>
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
  bool m_memory;
public:

  Boolean (const std::string& id, const bool& value)
    : Condition(id), m_value(value), m_memory(value) { }
  void set(const bool& value) { m_value = value; }
  void toggle() { m_value = !m_value; }
  virtual bool value() const { return m_value; }

  void begin_temporary_true() { m_memory = m_value; m_value = true; }
  void end_temporary_true() { m_value = m_memory; }
};

typedef std::shared_ptr<Boolean> Boolean_handle;

template <typename T>
class Value_condition : public Condition
{
  Value_handle<T> m_handle;
  T m_value;

public:

  Value_condition (const std::string& id, Value_handle<T> handle, const T& value)
    : Condition(id), m_handle (handle), m_value (value)
  { }

  virtual bool value() const { return m_handle->value() == m_value; }
};

template <typename T>
using Value_condition_handle = std::shared_ptr<Value_condition<T> >;

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

class Not : public Condition
{
  Condition_handle m_value;
  
public:

  Not (const std::string& id, Condition_handle value)
    : Condition(id), m_value(value) { }
  virtual bool value() const { return !(m_value->value()); }
};

typedef std::shared_ptr<Not> Not_handle;

// Helpers
template <typename T>
Value_condition_handle<T> make_value_condition (Value_handle<T> handle, const T& value)
{
  return make_handle<Value_condition<T> >(handle->entity() + ":value_cond", handle, value);
}
inline And_handle make_and (Condition_handle h0, Condition_handle h1)
{
  return make_handle<And>(h0->entity() + "_" + h1->entity() + ":and", h0, h1);
}
inline Or_handle make_or (Condition_handle h0, Condition_handle h1)
{
  return make_handle<Or>(h0->entity() + "_" + h1->entity() + ":or", h0, h1);
}
inline Not_handle make_not (Condition_handle handle)
{
  return make_handle<Not>(handle->entity() + ":not", handle);
}

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITION_H
