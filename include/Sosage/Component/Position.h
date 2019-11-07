#ifndef SOSAGE_COMPONENT_POSITION_H
#define SOSAGE_COMPONENT_POSITION_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/geometry.h>

#include <vector>

namespace Sosage::Component
{


class Position : public Base
{
  Point m_pos;
  std::shared_ptr<Position> m_ref;

public:

  Position (const std::string& id, const Point& coord,
            std::shared_ptr<Position> ref = std::shared_ptr<Position>());

  Point value () const
  {
    if (m_ref != std::shared_ptr<Position>())
      return m_pos + Vector(m_ref->value());
    return m_pos;
  }

  void set (const Point& p)
  {
    m_pos = p;
  }
};
typedef std::shared_ptr<Position> Position_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_POSITION_H
