/*
  [src/Sosage/Component/Animation.cpp]
  Specialization of Image for animated objects.

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

#include <Sosage/Component/Animation.h>

namespace Sosage::Component
{

Animation::Animation (const std::string& entity, const std::string& component,
                      const std::string& file_name, int z,
                      int width_subdiv, int height_subdiv, bool loop,
                      const Collision_type& collision, bool with_highlight)
  : Image(entity, component, file_name, z, collision, with_highlight)
  , m_width_subdiv (width_subdiv)
  , m_height_subdiv (height_subdiv)
  , m_current(0)
  , m_loop(loop)
{
  reset();
}

int Animation::width_subdiv() const
{
  return m_width_subdiv;
}

int Animation::height_subdiv() const
{
  return m_height_subdiv;
}

const std::vector<Animation::Frame>& Animation::frames() const
{
  return m_frames;
}

std::vector<Animation::Frame>& Animation::frames()
{
  return m_frames;
}

bool Animation::loop() const
{
  return m_loop;
}

bool Animation::animated() const
{
  return m_frames.size() > 1;
}

int Animation::reset (bool all_frames, int duration)
{
  int out = 0;
  
  m_frames.clear();
  if (all_frames)
    for (int i = 0; i < m_height_subdiv; ++ i)
      for (int j = 0; j < m_width_subdiv; ++ j)
      {
        m_frames.push_back ({j,i,duration});
        out += 1;
      }
  else
  {
    m_frames.push_back ({0,0,duration});
    out = 1;
  }
  m_current = 0;

  return out;
}

int Animation::xmin() const
{
  int width = Core::Graphic::width(this->core());
  int w = width / m_width_subdiv;
  return w * m_frames[m_current].x;
}

int Animation::xmax() const
{
  int width = Core::Graphic::width(this->core());
  int w = width / m_width_subdiv;
  return w * (m_frames[m_current].x + 1);
}

int Animation::ymin() const
{
  int height = Core::Graphic::height(this->core());
  int h = height / m_height_subdiv;
  return h * m_frames[m_current].y;
}

int Animation::ymax() const
{
  int height = Core::Graphic::height(this->core());
  int h = height / m_height_subdiv;
  return h * (m_frames[m_current].y + 1);
}

const Animation::Frame& Animation::current_frame() const
{
 return m_frames[m_current];
}

bool Animation::next_frame()
{
  if (++ m_frames[m_current].ellapsed == m_frames[m_current].duration)
  {
    m_frames[m_current].ellapsed = 0;
    if (++ m_current == m_frames.size())
    {
      m_current = 0;
      if (!m_loop)
        return false;
    }
  }
  return true;
}

} // namespace Sosage::Component
