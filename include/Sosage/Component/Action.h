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

#include <Sosage/Component/Base.h>

#include <functional>
#include <set>
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

    Step (const std::string& function, const std::vector<std::string>& args);
    const std::string& function() const;
    const std::vector<std::string>& args() const;
    std::string to_string() const;
  };

  using Timed_handle = std::pair<double, Handle>;

private:

  std::vector<Step> m_steps;
  std::set<Timed_handle> m_timed;
  std::size_t m_next_step;
  bool m_on;
  bool m_still_waiting; // for this world to stop hating

public:

  Action (const std::string& id);
  void clear();
  void add (const std::string& function, const std::vector<std::string>& args);
  void launch();
  void stop();
  bool on() const;
  std::string str() const;
  std::size_t size() const;
  std::vector<Step>::const_iterator begin() const;
  std::vector<Step>::const_iterator end() const;
  const std::set<Timed_handle>& scheduled() const;
  void schedule (double time, Handle h);
  void update_scheduled (const std::function<bool(Timed_handle)>& predicate);
  bool ready() const;
  const Step& next_step();
  const Step& last_step();
};

using Action_handle = std::shared_ptr<Action>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_ACTION_H
