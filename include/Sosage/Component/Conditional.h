/*
  [include/Sosage/Component/Conditional.h]
  Access different components depending on a condition.

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

#ifndef SOSAGE_COMPONENT_CONDITIONAL_H
#define SOSAGE_COMPONENT_CONDITIONAL_H

#include <Sosage/Component/Base.h>
#include <Sosage/Component/Condition.h>
#include <Sosage/Utils/conversions.h>

#include <unordered_map>
#include <vector>

namespace Sosage::Component
{

class Conditional_base : public Base
{
public:
  Conditional_base (const std::string& entity, const std::string& component);
  virtual Handle get() const = 0;
};

using Conditional_base_handle = std::shared_ptr<Conditional_base>;

class Conditional : public Conditional_base
{
  Condition_handle m_condition;
  Handle m_if_true;
  Handle m_if_false;

public:

  Conditional (const std::string& entity, const std::string& component,
               Condition_handle condition,
               Handle if_true,
               Handle if_false = Handle());
  virtual ~Conditional();
  virtual Handle get() const;

  STR_NAME("Conditional");
  STR_SUB(return component_str(m_condition, indent+1, "Condition = ")
          + component_str(m_if_true, indent+1, "If true = ")
          + component_str(m_if_true, indent+1, "If true = "););
};

using Conditional_handle = std::shared_ptr<Conditional>;

template <typename T>
class Simple_conditional : public Conditional_base
{
  Simple_handle<T> m_simple;
  std::unordered_map<T, Handle> m_handles;

public:

  Simple_conditional (const std::string& entity, const std::string& component,
                      Simple_handle<T> simple)
    : Conditional_base(entity, component)
    , m_simple (simple)
  { }

  virtual ~Simple_conditional()
  {
    m_simple = Simple_handle<T>();
    m_handles.clear();
  }

  void add (const T& s, Handle h)
  {
    m_handles.insert (std::make_pair (s, h));
  }

  void set (const T& s, Handle h)
  {
    auto iter = m_handles.find(s);
    dbg_check(iter != m_handles.end(), "Value " + to_string(s) + " not found in conditional " + str());
    iter->second = h;
  }

  virtual Handle get() const
  {
    auto iter
      = m_handles.find(m_simple->value());
    if (iter == m_handles.end())
      return Handle();
    return iter->second;
  }

  STR_NAME("Simple_conditional");
  STR_VALUE(to_string(m_simple->value()));
  STR_SUB(
      std::string out;
  for (const auto& h : m_handles)
      out += component_str(h.second, indent+1, "If " + to_string(h.first) + " = ");
  return out;
  );
};

template <typename T>
using Simple_conditional_handle = std::shared_ptr<Simple_conditional<T>>;

using String_conditional = Simple_conditional<std::string>;
using String_conditional_handle = std::shared_ptr<String_conditional>;

class Random_conditional : public Conditional_base
{
  std::vector<Handle> m_handles;

public:

  Random_conditional (const std::string& entity, const std::string& component);
  virtual ~Random_conditional();
  void add (Handle h);
  virtual Handle get() const;
};

using Random_conditional_handle = std::shared_ptr<Random_conditional>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITIONAL_H
