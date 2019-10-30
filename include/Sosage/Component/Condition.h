#ifndef SOSAGE_COMPONENT_CONDITION_H
#define SOSAGE_COMPONENT_CONDITION_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class Condition : public Base
{
public:

  Condition (const std::string& id) : Base(id) { }

  virtual bool value() const = 0;
};

typedef std::shared_ptr<Condition> Condition_handle;

class Boolean : public Condition
{
  bool m_value;
public:

  Boolean (const std::string& id, const bool& value)
    : Condition(id), m_value(value) { }

  void set(const bool& value) { m_value = value; }
  virtual bool value() const { return m_value; }
};

typedef std::shared_ptr<Boolean> Boolean_handle;

class And : public Condition
{
  std::pair<Boolean_handle, Boolean_handle> m_values;
  
public:

  And (const std::string& id,
       Boolean_handle first, Boolean_handle second)
    : Condition(id), m_values(first, second) { }

  virtual bool value() const { return (m_values.first->value() && m_values.second->value()); }
};

typedef std::shared_ptr<And> And_handle;

class Or : public Condition
{
  std::pair<Boolean_handle, Boolean_handle> m_values;
  
public:

  Or (const std::string& id,
      Boolean_handle first, Boolean_handle second)
    : Condition(id), m_values(first, second) { }

  virtual bool value() const { return (m_values.first->value() || m_values.second->value()); }
};

typedef std::shared_ptr<And> And_handle;


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITION_H
