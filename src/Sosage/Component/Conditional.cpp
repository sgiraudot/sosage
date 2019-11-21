#include <Sosage/Component/Conditional.h>

namespace Sosage::Component
{

Conditional::Conditional (const std::string& id,
                          Condition_handle condition,
                          Handle if_true,
                          Handle if_false)
  : Base(id)
  , m_condition (condition)
  , m_if_true (if_true)
  , m_if_false (if_false)
{ }

Conditional::~Conditional()
{
  m_condition = Condition_handle();
  m_if_true = Handle();
  m_if_false = Handle();
}

std::string Conditional::str() const
{
  std::string ift = "[" + (m_if_true ? m_if_true->str() : "NULL") + "]";
  std::string iff = "[" + (m_if_false ? m_if_false->str() : "NULL") + "]";
  return this->id() + " -> " + m_condition->id() + " ? "
    + ift + " : " + iff;
}

Handle Conditional::get()
{
  return (m_condition->value() ? m_if_true : m_if_false);
}

State_conditional::State_conditional (const std::string& id,
                                      State_handle state)
  : Base(id)
  , m_state (state)
{ }

State_conditional::~State_conditional()
{
  m_state = State_handle();
  m_handles.clear();
}

std::string State_conditional::str() const
{
  return this->id() + " -> " + m_state->id() + " ? "
    + get()->id();
}

void State_conditional::add (const std::string& state, Handle h)
{
  m_handles.insert (std::make_pair (state, h));
}
  
Handle State_conditional::get() const
{
  auto iter
    = m_handles.find(m_state->value());
  check (iter != m_handles.end(), "Cannot find state " + m_state->value());
  return iter->second;
}


} // namespace Sosage::Component
