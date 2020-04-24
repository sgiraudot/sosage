#include <Sosage/Component/Conditional.h>

namespace Sosage::Component
{

Variable::Variable (const std::string& id, Handle target)
  : Conditional_base(id), m_target (target)
{ }

Conditional::Conditional (const std::string& id,
                          Condition_handle condition,
                          Handle if_true,
                          Handle if_false)
  : Conditional_base(id)
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

Handle Conditional::get() const
{
  return (m_condition->value() ? m_if_true : m_if_false);
}

State_conditional::State_conditional (const std::string& id,
                                      State_handle state)
  : Conditional_base(id)
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
    + (get() ? get()->id() : "NULL");
}

void State_conditional::add (const std::string& state, Handle h)
{
  m_handles.insert (std::make_pair (state, h));
}
  
Handle State_conditional::get() const
{
  auto iter
    = m_handles.find(m_state->value());
  if (iter == m_handles.end())
    return Handle();
//  check (iter != m_handles.end(), "Cannot find state " + m_state->value() + " in " + id());
  return iter->second;
}

Random_conditional::Random_conditional (const std::string& id)
  : Conditional_base(id)
  , m_total(0)
{ }

Random_conditional::~Random_conditional()
{
  m_handles.clear();
}

std::string Random_conditional::str() const
{
  return this->id() + " -> " + get()->id();
}

void Random_conditional::add (double probability, Handle h)
{
  m_handles.push_back (std::make_pair (probability, h));
  m_total += probability;
}
  
Handle Random_conditional::get() const
{
  double random = m_total * (rand() / double(RAND_MAX));
  double accu = 0;

  std::size_t idx = 0;
  while (random >= accu && idx < m_handles.size())
  {
    accu += m_handles[idx].first;
    ++ idx;
  }

  return m_handles[idx-1].second;
}


} // namespace Sosage::Component
