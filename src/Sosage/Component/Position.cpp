#include <Sosage/Component/Position.h>

namespace Sosage::Component
{

Position::Position (const std::string& id, const Point& point, std::shared_ptr<Position> ref)
  : Base(id), m_pos (point), m_ref (ref)
{ }

} // namespace Sosage::Component
