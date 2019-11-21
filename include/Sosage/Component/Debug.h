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

  Debug (const std::string& id, const Content& content, const Clock& clock);
  virtual ~Debug();
  std::string debug_str() const;
};

typedef std::shared_ptr<Debug> Debug_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_DEBUG_H
