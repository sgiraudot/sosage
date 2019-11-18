#ifndef SOSAGE_COMPONENT_DEBUG_H
#define SOSAGE_COMPONENT_DEBUG_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Content.h>
#include <Sosage/Utils/time.h>

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

  std::string str() const
  {
    std::string out = "[Debug info]\n";
    out += "FPS = " + std::to_string(m_clock.fps()) + "\n";
    out += "CPU = " + std::to_string(m_clock.cpu()) + "%\n\n";

    out += std::to_string(m_content.size()) + " components in memory:\n";
    for (const auto& c : m_content)
      out += " * " + c->id() + "(" + typeid(*c).name() + ")\n";

    return out;
  }

};

typedef std::shared_ptr<Debug> Debug_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_DEBUG_H
