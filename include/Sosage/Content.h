#ifndef SOSAGE_CONTENT_H
#define SOSAGE_CONTENT_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Component/cast.h>
#include <Sosage/Utils/error.h>

namespace Sosage
{

class Content
{
private:

  Component::Handle_set m_data;
public:

  Content ();

  void clear() { m_data.clear(); }

  std::size_t size() const { return m_data.size(); }

  template <typename T>
  void set (const std::shared_ptr<T>& t)
  {
    Component::Handle_set::iterator iter = m_data.find(t);
    if (iter != m_data.end())
      m_data.erase(iter);
    m_data.insert(t);
  }
  
  template <typename T, typename ... Args>
  std::shared_ptr<T> set (Args&& ... args)
  {
    std::shared_ptr<T> new_component = std::make_shared<T>(args...);
    set (new_component);
    return new_component;
  }

  Component::Handle_set::const_iterator begin() const { return m_data.begin(); }
  Component::Handle_set::const_iterator end() const { return m_data.end(); }

  void remove (const std::string& key)
  {
    Component::Handle_set::iterator iter = m_data.find(std::make_shared<Component::Base>(key));
    check (iter != m_data.end(), "Entity " + key + " doesn't exist");
    m_data.erase(iter);
  }

  template <typename T>
  std::shared_ptr<T> request (const std::string& key)
  {
    Component::Handle_set::iterator iter = m_data.find(std::make_shared<Component::Base>(key));
    if (iter == m_data.end())
      return std::shared_ptr<T>();

    std::shared_ptr<T> out = Component::cast<T>(*iter);
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
