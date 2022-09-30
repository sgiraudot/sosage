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
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/error.h>

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

std::string Action::Step::to_string() const
{
  std::string out = m_function + ":[";
  for (std::size_t i = 0; i < m_args.size(); ++ i)
  {
    out += m_args[i];
    if (i != m_args.size() - 1)
      out += ", ";
  }

  return out + "]";
}

Action::Action (const std::string& entity, const std::string& component)
  : Base (entity, component), m_next_step(0), m_on(false), m_still_waiting(false)
{ }

void Action::clear()
{
 m_steps.clear();
 stop();
}

void Action::add (const std::string& function, const std::vector<std::string>& args)
{
  m_steps.push_back (Step (function, args));
}

void Action::launch()
{
  m_on = true;
  m_next_step = 0;
}

void Action::stop()
{
  m_on = false;
  m_timed.clear();
}

bool Action::on() const
{
 return m_on;
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

const std::set<Action::Timed_handle>& Action::scheduled() const
{
  return m_timed;
}

void Action::schedule (double time, Handle h)
{
  m_timed.insert (std::make_pair (time, h));
}

void Action::reset_scheduled()
{
  debug << this->str() << " reset scheduled" << std::endl;
  m_timed.clear();
}

void Action::update_scheduled (const std::function<bool(Timed_handle)>& predicate)
{
  m_still_waiting = true;

  if (m_timed.empty())
    return;

  std::set<Timed_handle> new_timed_handle;
  for (const Timed_handle& th : m_timed)
    if (predicate(th))
      new_timed_handle.insert(th);
    else if (th.second->entity() == "wait")
    {
//      debug << "Stop waiting" << std::endl;
      m_still_waiting = false;
    }

  m_timed.swap(new_timed_handle);
}

bool Action::ready() const
{
  return !m_still_waiting || m_timed.empty();
}

const Action::Step& Action::next_step()
{
  check (m_next_step < m_steps.size(), "Trying to access step " + to_string(m_next_step)
         + " of action " + Base::str() + " of size " + to_string(m_steps.size()));

  // For debug porposes, we can skip some parts between "skip" and "include":
  if (m_steps[m_next_step].function() == "function_skip")
    do
    {
      ++ m_next_step;
    }
    while (m_steps[m_next_step - 1].function() != "function_include");

  const Step& out = m_steps[m_next_step ++];
  if (m_next_step == m_steps.size())
    m_on = false;
  return out;
}

const Action::Step& Action::first_step()
{
  return m_steps.front();
}

const Action::Step& Action::last_step()
{
  return m_steps.back();
}

} // namespace Sosage::Component
