#ifndef SOSAGE_COMPONENT_STATE_H
#define SOSAGE_COMPONENT_STATE_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class State : public Base
{
  std::string m_value;
  
public:

  State (const std::string& id)
    : Base(id)
  { }

  virtual ~State() {} 

  const std::string& value() const { return m_value; }
  void set (const std::string& value) { m_value = value; }

};

typedef std::shared_ptr<State> State_handle;

} // namespace Sosage::State

#endif // SOSAGE_COMPONENT_STATE_H
