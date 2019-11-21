#include <Sosage/Component/Text.h>

namespace Sosage::Component
{

Text::Text (const std::string& id, const std::string& value)
  : Base (id), m_value (value)
{ }

} // namespace Sosage::Component
