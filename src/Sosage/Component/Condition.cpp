#include <Sosage/Component/Condition.h>

namespace Sosage::Component
{

Condition::Condition (const std::string& id)
 : Base(id)
{ }

std::string Condition::str() const
{
  return this->id() + " " + (value() ? "TRUE" : "FALSE");
}


} // namespace Sosage::Component
