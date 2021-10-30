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

#include <unordered_map>
#include <vector>

namespace Sosage::Component
{

class Conditional_base : public Base
{
public:
  Conditional_base (const std::string& id);
  virtual Handle get() const = 0;
};

using Conditional_base_handle = std::shared_ptr<Conditional_base>;

class Conditional : public Conditional_base
{
  Condition_handle m_condition;
  Handle m_if_true;
  Handle m_if_false;
  
public:

  Conditional (const std::string& id,
               Condition_handle condition,
               Handle if_true,
               Handle if_false = Handle());
  virtual ~Conditional();
  virtual std::string str() const;
  virtual Handle get() const;
};

using Conditional_handle = std::shared_ptr<Conditional>;

class String_conditional : public Conditional_base
{
  String_handle m_state;
  std::unordered_map<std::string, Handle> m_handles;
  
public:

  String_conditional (const std::string& id,
                      String_handle state);
  virtual ~String_conditional();
  virtual std::string str() const;
  void add (const std::string& state, Handle h);
  void set (const std::string& state, Handle h);
  virtual Handle get() const;
};

using String_conditional_handle = std::shared_ptr<String_conditional>;

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

using Random_conditional_handle = std::shared_ptr<Random_conditional>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITIONAL_H
