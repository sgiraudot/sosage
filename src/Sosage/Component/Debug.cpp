/*
  [src/Sosage/Component/Debug.cpp]
  Debug information on screen (FPS, etc.).

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

#include <Sosage/Component/Debug.h>

#include <algorithm>
#include <vector>

namespace Sosage::Component
{

Debug::Debug (const std::string& id, const Content& content, const Clock& clock)
  : Boolean(id, false), m_content (content), m_clock (clock)
{ }

Debug::~Debug()
{ }

std::string Debug::debug_str() const
{
  std::string out = "[Debug info]\n";
  out += "FPS = " + std::to_string(m_clock.fps()) + "\n";
  out += "CPU = " + std::to_string(m_clock.cpu()) + "%\n\n";

  out += std::to_string(m_content.size()) + " components in memory:";

  return out;
  

  std::vector<Component::Handle> components;
  components.reserve (m_content.size());
  std::copy (m_content.begin(), m_content.end(), std::back_inserter (components));
  std::sort (components.begin(), components.end(),
             [&](const Component::Handle& a, const Component::Handle& b) -> bool
             {
               return a->id() < b->id();
             });


  std::string latest = "";
  for (const auto& c : components)
  {
    if (!c)
      continue;

    std::string entity = c->entity();
    std::cerr << entity << std::endl;

    if (entity != latest)
    {
      out += "\n *";
      latest = entity;
    }
    out += " [" + c->str() + "]";

  }

  return out;
}


}
