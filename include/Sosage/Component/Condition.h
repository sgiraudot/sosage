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

#include <Sosage/Component/Base.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Utils/conversions.h>

namespace Sosage::Component
{

class Condition : public Value<bool>
{
public:

  Condition (const std::string& entity, const std::string& component);
  virtual bool value() const = 0;
  virtual std::string str() const;
};

using Condition_handle = std::shared_ptr<Condition>;

class Boolean : public Condition
{
public:
  using const_reference = bool;
  using value_type = bool;

private:
  bool m_value;
  bool m_memory;
public:

  Boolean (const std::string& entity, const std::string& component, const bool& value);
  void set(const bool& value);
  void toggle();
  virtual bool value() const;
  void begin_temporary_true();
  void end_temporary_true();

  STR_NAME("Boolean")
  STR_VALUE(to_string(m_value));
};

using Boolean_handle = std::shared_ptr<Boolean>;

template <typename T>
class Value_condition : public Condition
{
  Value_handle<T> m_handle;
  T m_value;

public:

  Value_condition (const std::string& entity, const std::string& component,
                   Value_handle<T> handle, const T& value)
    : Condition(entity, component), m_handle (handle), m_value (value)
  { }

  virtual bool value() const { return m_handle->value() == m_value; }
  STR_NAME("Value_condition")
  STR_VALUE(to_string(m_value));
  STR_SUB(return component_str(m_handle, indent+1, "Reference = "););
};

template <typename T>
using Value_condition_handle = std::shared_ptr<Value_condition<T> >;

template <typename T>
class Simple_condition : public Condition
{
  Simple_handle<T> m_handle;
  T m_value;

public:

  Simple_condition (const std::string& entity, const std::string& component,
                    Simple_handle<T> handle, const T& value)
    : Condition(entity, component), m_handle (handle), m_value (value)
  { }

  virtual bool value() const { return m_handle->value() == m_value; }

  STR_NAME("Simple_condition")
  STR_SUB(return component_str(m_handle, indent+1, "Reference = "););
};

template <typename T>
using Simple_condition_handle = std::shared_ptr<Simple_condition<T> >;

class And : public Condition
{
  std::pair<Condition_handle, Condition_handle> m_values;
  
public:

  And (const std::string& entity, const std::string& component,
       Condition_handle first, Condition_handle second);
  virtual bool value() const;

  STR_NAME("And");
  STR_SUB(return component_str(m_values.first, indent+1, "A = ")
          + component_str(m_values.second, indent+1, "B = "););
};

using And_handle = std::shared_ptr<And>;

class Or : public Condition
{
  std::pair<Condition_handle, Condition_handle> m_values;
  
public:

  Or (const std::string& entity, const std::string& component,
      Condition_handle first, Condition_handle second);
  virtual bool value() const;
  STR_NAME("Or");
  STR_SUB(return component_str(m_values.first, indent+1, "A = ")
          + component_str(m_values.second, indent+1, "B = "););
};

using Or_handle = std::shared_ptr<Or>;

class Not : public Condition
{
  Condition_handle m_value;
  
public:

  Not (const std::string& entity, const std::string& component, Condition_handle value);
  virtual bool value() const;
  STR_NAME("Not");
  STR_SUB(return component_str(m_value, indent+1, "Negate = "););
};

using Not_handle = std::shared_ptr<Not>;

// Helpers
template <typename T>
Value_condition_handle<T> make_value_condition (Value_handle<T> handle, const T& value)
{
  return make_handle<Value_condition<T> >(handle->entity(), "value_cond", handle, value);
}
template <typename T>
Simple_condition_handle<T> make_simple_condition (Simple_handle<T> handle, const T& value)
{
  return make_handle<Simple_condition<T> >(handle->entity(), "simple_cond", handle, value);
}

And_handle make_and (Condition_handle h0, Condition_handle h1);
Or_handle make_or (Condition_handle h0, Condition_handle h1);
Not_handle make_not (Condition_handle handle);

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITION_H
