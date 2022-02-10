/*
  [src/Sosage/System/Base.cpp]
  Virtual basis for all systems.

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

#include <Sosage/Component/Locale.h>
#include <Sosage/System/Base.h>

namespace Sosage::System
{

Base::Base (Content& content)
  : m_content (content)
{ }

Base::~Base() { }

void Base::init() { }

#ifdef SOSAGE_PROFILE

void Base::start_timer ()
{
  m_start = Time::now();
}

void Base::stop_timer (const std::string& id)
{
  set<Component::Int>(id + ":time", Time::now() - m_start);
}

#else
void Base::start_timer () { }
void Base::stop_timer (const char*) { }
#endif

Component::Handle_set Base::components (const std::string& s)
{
  return m_content.components(s);
}

void Base::remove (const std::string& key, bool optional)
{
  m_content.remove(key, optional);
}

void Base::emit (const std::string& signal)
{
  m_content.emit (signal);
}

bool Base::receive (const std::string& signal)
{
  return m_content.receive(signal);
}

Component::Status_handle Base::status()
{
  return get<Component::Status>(GAME__STATUS);
}

const std::string& Base::locale (const std::string& line)
{
  if (auto l = request<Component::Locale>("Game:locale"))
    return l->get(line);
  // else
  return line;
}

const std::string& Base::locale_get (const std::string& id)
{
  return locale (value<Component::String>(id));
}

} // namespace Sosage::System
