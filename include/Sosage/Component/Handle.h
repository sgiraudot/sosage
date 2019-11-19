#ifndef SOSAGE_COMPONENT_HANDLE_H
#define SOSAGE_COMPONENT_HANDLE_H

#include <memory>
#include <unordered_set>

namespace Sosage::Component
{

class Base
{
  std::string m_id; // entity:component or entity:state:component
  
public:
  Base(const std::string& id) : m_id (id) { }
  virtual ~Base() { }

  const std::string& id() const { return m_id; }

  std::string entity()
  {
    return std::string (m_id.begin(), m_id.begin() + m_id.find_first_of(':'));
  }

  std::string component()
  {
    return std::string (m_id.begin() + m_id.find_last_of(':'), m_id.end());
  }

  virtual std::string str() const { return m_id; }

  void set_id (const std::string& id) { m_id = id; }
};

typedef std::shared_ptr<Base> Handle;

template <typename T, typename ... Args>
std::shared_ptr<T> make_handle (Args ... args)
{
  return std::make_shared<T>(args...);
}

struct Hash_ids
{
  std::size_t operator() (const Handle& h) const
  {
    return std::hash<std::string>()(h->id());
  }
};

struct Equal_ids
{
  bool operator() (const Handle& a,
                   const Handle& b) const
  {
    return (a->id() == b->id());
  }
};

typedef std::unordered_set<Handle, Hash_ids, Equal_ids> Handle_set;
    
} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_H
