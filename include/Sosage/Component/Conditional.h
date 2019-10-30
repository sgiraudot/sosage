#ifndef SOSAGE_COMPONENT_CONDITIONAL_H
#define SOSAGE_COMPONENT_CONDITIONAL_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class Conditional : public Base
{
  Condition_handle m_condition;
  Handle m_if_true;
  Handle m_if_false;
  
public:

  Conditional (const std::string& id,
               Condition_handle condition,
               Handle if_true,
               Handle if_false)
    : Base(id)
    , m_condition (condition)
    , m_if_true (if_true)
    , m_if_false (if_false)
  { }

  virtual ~Conditional()
  {
    m_condition = Condition_handle();
    m_if_true = Handle();
    m_if_false = Handle();
  }

  Handle get()
  {
    return (m_condition->value() ? m_if_true : m_if_false);
  }

};

typedef std::shared_ptr<Conditional> Conditional_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITIONAL_H
