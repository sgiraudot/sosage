#include <Sosage/Component/Position.h>

namespace Sosage::Component
{

Position::Position (const std::string& id, const Point& point)
  : Base(id), m_pos (point)
{ }

} // namespace Sosage::Component
