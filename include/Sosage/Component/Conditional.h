#ifndef SOSAGE_COMPONENT_CONDITIONAL_H
#define SOSAGE_COMPONENT_CONDITIONAL_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Component/State.h>
#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/error.h>

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

class State_conditional : public Base
{
  State_handle m_state;
  Handle_set m_handles;
  
public:

  State_conditional (const std::string& id,
                     State_handle state)
    : Base(id)
    , m_state (state)
  { }

  virtual ~State_conditional()
  {
    m_state = State_handle();
    m_handles.clear();
  }

  Handle get()
  {
    Component::Handle_set::iterator iter
      = m_handles.find(std::make_shared<Component::Base>(m_state->value()));
    check (iter != m_handles.end(), "Cannot find state " + m_state->value());
    return *iter;
  }

};

typedef std::shared_ptr<State_conditional> State_conditional_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITIONAL_H
