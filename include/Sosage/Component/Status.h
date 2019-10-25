#ifndef SOSAGE_COMPONENT_STATUS_H
#define SOSAGE_COMPONENT_STATUS_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

enum STATUS { IDLE, EXIT };

class Status : public Base
{
  STATUS m_value;
  
public:

  Status (const std::string& id,const STATUS& value) : Base(id), m_value (value) { }

  bool exit() const { return (m_value == EXIT); }
};

typedef std::shared_ptr<Status> Status_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_STATUS_H
