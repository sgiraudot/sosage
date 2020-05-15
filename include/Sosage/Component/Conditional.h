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

#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Handle.h>

#include <unordered_map>
#include <vector>

namespace Sosage::Component
{

class Conditional_base : public Base
{
public:
  Conditional_base (const std::string& id) : Base(id) { }

  virtual Handle get() const = 0;
};

typedef std::shared_ptr<Conditional_base> Conditional_base_handle;

class Variable : public Conditional_base
{
  Handle m_target;

public:

  Variable (const std::string& id, Handle target);

  void set (Handle target) { m_target = target; }
  virtual Handle get() const { return m_target; }
};

typedef std::shared_ptr<Variable> Variable_handle;

class Conditional : public Conditional_base
{
  Condition_handle m_condition;
  Handle m_if_true;
  Handle m_if_false;
  
public:

  Conditional (const std::string& id,
               Condition_handle condition,
               Handle if_true,
               Handle if_false);

  virtual ~Conditional();
  virtual std::string str() const;
  virtual Handle get() const;
};

typedef std::shared_ptr<Conditional> Conditional_handle;

class State_conditional : public Conditional_base
{
  String_handle m_state;
  std::unordered_map<std::string, Handle> m_handles;
  
public:

  State_conditional (const std::string& id,
                     String_handle state);
  virtual ~State_conditional();
  virtual std::string str() const;
  void add (const std::string& state, Handle h);
  virtual Handle get() const;
};

typedef std::shared_ptr<State_conditional> State_conditional_handle;

class Random_conditional : public Conditional_base
{
  String_handle m_state;
  std::vector<std::pair<double, Handle> > m_handles;
  double m_total;
  
public:

  Random_conditional (const std::string& id);
  virtual ~Random_conditional();
  virtual std::string str() const;
  void add (double probability, Handle h);
  virtual Handle get() const;
};

typedef std::shared_ptr<Random_conditional> Random_conditional_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITIONAL_H
