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
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Status.h>

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
  out += "CPU = " + std::to_string(m_clock.cpu()) + "%\n";
  out += m_content.get<Component::Status>(GAME__STATUS)->str() + "\n\n";

  out += std::to_string(m_content.size()) + " components in memory\n";

  const std::string& player = m_content.get<Component::String>("Player:name")->value();
  auto img = m_content.get<Component::Image>(player + "_body:image");
  auto pos = m_content.get<Component::Position>(player + "_body:position");

  out += "Player position = [" + std::to_string(pos->value().x())
         + ", " + std::to_string(pos->value().y()) + ", " + std::to_string(img->z()) + "]\n";

  return out;
}

} // namespace Sosage::Component
