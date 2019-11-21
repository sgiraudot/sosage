#include <Sosage/Component/Position.h>

namespace Sosage::Component
{

Position::Position (const std::string& id, const Point& point)
  : Base(id), m_pos (point)
{ }

std::string Position::str() const
{
  return this->id() + " (" + std::to_string (m_pos.x())
    + ";" + std::to_string(m_pos.y()) + ")";
}
  
} // namespace Sosage::Component
