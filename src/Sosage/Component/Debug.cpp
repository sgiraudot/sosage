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
#include <Sosage/Component/Hints.h>

#include <algorithm>
#include <vector>

namespace Sosage::Component
{

Debug::Debug (const std::string& id, Content& content, const Clock& clock)
  : Boolean(id, false), m_content (content), m_clock (clock)
{ }

Debug::~Debug()
{ }

std::string Debug::debug_str()
{
  std::string out = "[Debug info]\n";
  out += "FPS = " + std::to_string(m_clock.fps()) + "\n";
  out += "CPU = " + std::to_string(m_clock.cpu()) + "%\n\n";

  out += std::to_string(m_content.size()) + " components in memory\n";
  out += "Next hint: " + m_content.get<Component::Hints>("Game:hints")->next() + "\n";

  return out;
}


}
