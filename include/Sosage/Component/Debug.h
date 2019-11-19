#ifndef SOSAGE_COMPONENT_DEBUG_H
#define SOSAGE_COMPONENT_DEBUG_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Content.h>
#include <Sosage/Utils/time.h>

#include <algorithm>

namespace Sosage::Component
{

class Debug : public Boolean
{
private:

  const Content& m_content;
  const Clock& m_clock;
  
public:

  Debug (const std::string& id, const Content& content, const Clock& clock)
    : Boolean(id, false), m_content (content), m_clock (clock) { }
  virtual ~Debug() { }

  std::string debug_str() const
  {
    std::string out = "[Debug info]\n";
    out += "FPS = " + std::to_string(m_clock.fps()) + "\n";
    out += "CPU = " + std::to_string(m_clock.cpu()) + "%\n\n";

    out += std::to_string(m_content.size()) + " components in memory:\n";

    std::vector<Component::Handle> components;
    components.reserve (m_content.size());
    std::copy (m_content.begin(), m_content.end(), std::back_inserter (components));
    std::sort (components.begin(), components.end(),
               [&](const Component::Handle& a, const Component::Handle& b) -> bool
               {
                 return a->id() < b->id();
               });
    
    for (const auto& c : components)
      out += " * " + c->str() + "\n";

    return out;
  }

};

typedef std::shared_ptr<Debug> Debug_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_DEBUG_H
