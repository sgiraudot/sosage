#ifndef SOSAGE_COMPONENT_PATH_H
#define SOSAGE_COMPONENT_PATH_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/geometry.h>

#include <vector>

namespace Sosage::Component
{


class Path : public Base
{
  std::vector<Point> m_steps;
  std::size_t m_current;

public:

  Path (const Point& coord);
  Path (std::vector<Point>& steps);
  std::size_t size() const { return m_steps.size(); }
  const Point& operator[] (const std::size_t& idx) const { return m_steps[idx]; }
  Point& operator[] (const std::size_t& idx) { return m_steps[idx]; }
  const std::size_t& current() const { return m_current; }
  std::size_t& current() { return m_current; }

};
typedef std::shared_ptr<Path> Path_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_PATH_H
