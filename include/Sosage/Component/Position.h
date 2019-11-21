#ifndef SOSAGE_COMPONENT_POSITION_H
#define SOSAGE_COMPONENT_POSITION_H

#include <Sosage/Component/Image.h>
#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/geometry.h>

#include <vector>

namespace Sosage::Component
{


class Position : public Base
{
  Point m_pos;

public:

  Position (const std::string& id, const Point& coord);
  virtual std::string str() const;
  Point value () const { return m_pos; }
  void set (const Point& p) { m_pos = p; }
};
typedef std::shared_ptr<Position> Position_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_POSITION_H
