#include <Sosage/Component/State.h>

namespace Sosage::Component
{

State::State (const std::string& id, const std::string& value)
  : Base(id), m_value (value)
{ }

State::~State()
{ }

std::string State::str() const
{
  return this->id() + " " + m_value;
}

} // namespace Sosage::Component
