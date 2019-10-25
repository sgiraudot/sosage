#ifndef SOSAGE_CONTENT_H
#define SOSAGE_CONTENT_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/error.h>

#include <unordered_set>

namespace Sosage
{

class Content
{
  struct Hash_ids
  {
    std::size_t operator() (const Component::Handle& h) const
    {
      return std::hash<std::string>()(h->id());
    }
  };

  struct Equal_ids
  {
    bool operator() (const Component::Handle& a,
                     const Component::Handle& b) const
    {
      return (a->id() == b->id());
    }
  };
    
public:

  typedef std::unordered_set<Component::Handle, Hash_ids, Equal_ids> Handle_set;
  
private:

  Handle_set m_data;
  Component::Handle m_finder;

public:

  Content ();

  std::size_t size() const { return m_data.size(); }

  template <typename T>
  void set (const std::shared_ptr<T>& t)
  {
    Handle_set::iterator iter = m_data.find(t);
    if (iter != m_data.end())
      m_data.erase(iter);
    m_data.insert(t);
  }
  
  template <typename T, typename ... Args>
  std::shared_ptr<T> set (Args ... args)
  {
    std::shared_ptr<T> new_component = std::make_shared<T>(args...);
    set (new_component);
    return new_component;
  }

  Handle_set::const_iterator begin() { return m_data.begin(); }
  Handle_set::const_iterator end() { return m_data.end(); }

  void remove (const std::string& key)
  {
    m_finder->set_id(key);
    Handle_set::iterator iter = m_data.find(m_finder);
    check (iter != m_data.end(), "Entity " + key + " doesn't exist");
    m_data.erase(iter);
  }

  template <typename T>
  std::shared_ptr<T> request (const std::string& key)
  {
    m_finder->set_id(key);
    Handle_set::iterator iter = m_data.find(m_finder);
    if (iter == m_data.end())
      return std::shared_ptr<T>();

    std::shared_ptr<T> out = Component::component_cast<T>(*iter);
    return out;
  }
  
  template <typename T>
  std::shared_ptr<T> get (const std::string& key)
  {
    std::shared_ptr<T> out = request<T>(key);
    check (out != std::shared_ptr<T>(), "Cannot find " + key);
    return out;
  }
  
};

} // namespace Sosage

#endif // SOSAGE_CONTENT_H
