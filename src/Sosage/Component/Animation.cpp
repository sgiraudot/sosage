#include <Sosage/Component/Animation.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Animation::Animation (const std::string& id, const std::string& file_name, int z,
                      int width_subdiv, int height_subdiv)
  : Image(id, file_name, z)
  , m_width_subdiv (width_subdiv)
  , m_height_subdiv (height_subdiv)
  , m_current(0)
{
  debug ("Creating animation " + file_name);
  reset();
}

void Animation::reset()
{
  m_frames.clear();
  m_frames.push_back (Frame(0,0,1));
  m_current = 0;
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


} // namespace Sosage::Component
