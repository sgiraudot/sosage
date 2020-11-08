/*
  [include/Sosage/Component/Action.h]
  The effects of a user-selected verb action ("open", "use", etc.).

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

#ifndef SOSAGE_COMPONENT_ACTION_H
#define SOSAGE_COMPONENT_ACTION_H

#include <Sosage/Component/Handle.h>

#include <cstdlib>
#include <vector>

namespace Sosage::Component
{

class Action : public Base
{
public:

  class Step
  {
  private:

    std::string m_function;
    std::vector<std::string> m_args;

  public:

    Step (const std::string& function, const std::vector<std::string>& args)
      : m_function (function), m_args (args)
    { }

    const std::string& function() const { return m_function; }
    const std::vector<std::string>& args() const { return m_args; }
  };

private:

  std::vector<Step> m_steps;

public:

  Action (const std::string& id);
  void add (const std::string& function, const std::vector<std::string>& args);
  std::string str() const;

  std::size_t size() const { return m_steps.size(); }
  std::vector<Step>::const_iterator begin() const { return m_steps.begin(); }
  std::vector<Step>::const_iterator end() const { return m_steps.end(); }
  const Step& operator[] (const std::size_t& idx) const { return m_steps[idx]; }

};

typedef std::shared_ptr<Action> Action_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_ACTION_H
