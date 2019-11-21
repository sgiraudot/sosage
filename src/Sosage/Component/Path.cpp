#include <Sosage/Component/Path.h>

namespace Sosage::Component
{

Path::Path (const std::string& id, const Point& point)
  : Base(id), m_steps (1, point), m_current(0)
{ }

Path::Path (const std::string& id, std::vector<Point>& steps)
  : Base(id), m_current(0)
{
  m_steps.swap(steps);
}

std::string Path::str() const
{
  std::string out = this->id();
  for (const Point& p : m_steps)
    out += " (" + std::to_string(p.x()) + ";" + std::to_string(p.y()) + ")";
  return out;
}

} // namespace Sosage::Component
