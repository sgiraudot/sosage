#include <Sosage/Component/State.h>

namespace Sosage::Component
{

State::State (const std::string& id)
  : Base(id)
{ }

State::~State()
{ }

std::string State::str() const
{
  return this->id() + " " + m_value;
}

} // namespace Sosage::Component
