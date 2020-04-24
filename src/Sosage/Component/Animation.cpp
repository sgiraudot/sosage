#include <Sosage/Component/Animation.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Animation::Animation (const std::string& id, const std::string& file_name, int z,
                      int width_subdiv, int height_subdiv, bool loop)
  : Image(id, file_name, z)
  , m_width_subdiv (width_subdiv)
  , m_height_subdiv (height_subdiv)
  , m_current(0)
  , m_loop(loop)
{
  reset();
}

int Animation::reset (bool all_frames)
{
  int out = 0;
  
  m_frames.clear();
  if (all_frames)
    for (std::size_t i = 0; i < m_width_subdiv; ++ i)
      for (std::size_t j = 0; j < m_height_subdiv; ++ j)
      {
        m_frames.push_back (Frame(i,j,1));
        out += 1;
      }
  else
  {
    m_frames.push_back (Frame(0,0,1));
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

bool Animation::next_frame()
{
  if (++ m_frames[m_current].ellapsed == m_frames[m_current].duration)
  {
    m_frames[m_current].ellapsed = 0;
    if (++ m_current == m_frames.size())
    {
      if (!m_loop)
        return false;
      else
        m_current = 0;
    }
  }
  return true;
}

} // namespace Sosage::Component
