#include <Sosage/Component/Action.h>

namespace Sosage::Component
{

Action::Action (const std::string& id)
  : Base (id)
{ }

void Action::add (const std::initializer_list<std::string>& content)
{
  m_steps.push_back (Step (content));
}

std::string Action::str() const
{
  std::string out = this->id() + ":\n";
  
  for (const Step& s : m_steps)
  {
    out += " * ";
    for (std::size_t i = 0; i < s.size(); ++ i)
      out += s.get(i) + " ";
    out += "\n";
  }

  return out;
}


} // namespace Sosage::Component
