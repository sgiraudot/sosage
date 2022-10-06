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

Debug::Debug (const std::string& entity, const std::string& component, Content& content, const Clock& clock)
  : Boolean(entity, component, false), m_content (content), m_clock (clock)
  , m_start(0), m_mean(0), m_nb(0)
{ }

Debug::~Debug()
{ }

std::string Debug::debug_str()
{
  std::string out = "[Debug info]\n";
  out += "FPS = " + std::to_string(int(std::round(m_clock.fps()))) + "Hz\n";
  out += "CPU = " + std::to_string(int(std::round(100. * m_cpu))) + "%\n";
  out += m_content.get<Component::Status>(GAME__STATUS)->str() + "\n\n";

  std::size_t nb_comp = 0;
  for (const auto& cmp : m_content)
    nb_comp += cmp.size();
  out += std::to_string(nb_comp) + " components in memory\n";

  if (auto player_cmp = m_content.request<Component::String>("Player", "name"))
  {
    const std::string& player = player_cmp->value();
    if (auto img = m_content.request<Component::Image>(player + "_body", "image"))
    {
      auto pos = m_content.get<Component::Position>(player + "_body", "position");

      out += "Player position = [" + std::to_string(pos->value().x())
             + ", " + std::to_string(pos->value().y()) + ", " + std::to_string(img->z()) + "]\n";
    }
  }
  return out;
}

void Debug::start_loop()
{
  m_start = m_clock.get();
}

void Debug::end_loop()
{
  double end = m_clock.get();
  m_mean += (end - m_start);
  ++ m_nb;
  if (m_nb == 60)
  {
    double time = m_mean / m_nb;
    m_mean = 0.;
    m_nb = 0;
    m_cpu = std::min(1., time / (1000 / m_clock.fps()));
  }
}

} // namespace Sosage::Component
