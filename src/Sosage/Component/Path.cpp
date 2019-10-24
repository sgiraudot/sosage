#include <Sosage/Component/Path.h>

namespace Sosage::Component
{

Path::Path (const Point& point)
  : m_steps (1, point), m_current(0)
{ }
Path::Path (std::vector<Point>& steps)
  : m_current(0)
{
  m_steps.swap(steps);
}

} // namespace Sosage::Component
