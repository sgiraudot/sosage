#ifndef SOSAGE_COMPONENT_HANDLE_H
#define SOSAGE_COMPONENT_HANDLE_H

#include <memory>

namespace Sosage::Component
{

class Base
{
  std::string m_id;
  
public:
  Base(const std::string& id) : m_id (id) { }
  virtual ~Base() { }

  const std::string& id() { return m_id; }

  std::string entity()
  {
    return std::string (m_id.begin(), m_id.begin() + m_id.find(":"));
  }

  void set_id (const std::string& id) { m_id = id; }
};

typedef std::shared_ptr<Base> Handle;

template <typename T>
inline std::shared_ptr<T> component_cast (Handle h)
{
  return std::dynamic_pointer_cast<T>(h);
}

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_H
