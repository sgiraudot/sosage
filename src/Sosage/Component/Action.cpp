/*
  [src/Sosage/Component/Action.cpp]
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

#include <Sosage/Component/Action.h>

namespace Sosage::Component
{

Action::Step::Step (const std::string& function, const std::vector<std::string>& args)
  : m_function (function), m_args (args)
{ }

const std::string& Action::Step::function() const
{
  return m_function;
}
const std::vector<std::string>& Action::Step::args() const
{
  return m_args;
}

Action::Action (const std::string& id)
  : Base (id)
{ }

void Action::add (const std::string& function, const std::vector<std::string>& args)
{
  m_steps.push_back (Step ("function_" + function, args));
}

std::string Action::str() const
{
  std::string out = this->id() + ":\n";
  
  for (const Step& s : m_steps)
  {
    out += " * " + s.function() + " ";
    for (std::size_t i = 0; i < s.args().size(); ++ i)
      out += s.args()[i] + " ";
    out += "\n";
  }

  return out;
}

std::size_t Action::size() const
{
  return m_steps.size();
}

std::vector<Action::Step>::const_iterator Action::begin() const
{
  return m_steps.begin();
}

std::vector<Action::Step>::const_iterator Action::end() const
{
  return m_steps.end();
}

const Action::Step& Action::operator[] (const std::size_t& idx) const
{
  return m_steps[idx];
}

} // namespace Sosage::Component
