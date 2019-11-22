#ifndef SOSAGE_COMPONENT_CONDITIONAL_H
#define SOSAGE_COMPONENT_CONDITIONAL_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Component/State.h>
#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/error.h>

#include <unordered_map>

namespace Sosage::Component
{

class Variable : public Base
{
  Handle m_target;

public:

  Variable (const std::string& id, Handle target);

  void set (Handle target) { m_target = target; }
  Handle get() { return m_target; }
};

typedef std::shared_ptr<Variable> Variable_handle;

class Conditional : public Base
{
  Condition_handle m_condition;
  Handle m_if_true;
  Handle m_if_false;
  
public:

  Conditional (const std::string& id,
               Condition_handle condition,
               Handle if_true,
               Handle if_false);

  virtual ~Conditional();
  virtual std::string str() const;
  Handle get();
};

typedef std::shared_ptr<Conditional> Conditional_handle;

class State_conditional : public Base
{
  State_handle m_state;
  std::unordered_map<std::string, Handle> m_handles;
  
public:

  State_conditional (const std::string& id,
                     State_handle state);
  virtual ~State_conditional();
  virtual std::string str() const;
  void add (const std::string& state, Handle h);
  Handle get() const;
};

typedef std::shared_ptr<State_conditional> State_conditional_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITIONAL_H
