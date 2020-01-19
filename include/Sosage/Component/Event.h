#ifndef SOSAGE_COMPONENT_EVENT_H
#define SOSAGE_COMPONENT_EVENT_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class Event : public Base
{
public:

  Event (const std::string& id) : Base(id) { }
  
};

typedef std::shared_ptr<Event> Event_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITION_H
