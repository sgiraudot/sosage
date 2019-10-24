#ifndef SOSAGE_CONTENT_H
#define SOSAGE_CONTENT_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/error.h>

#include <unordered_map>

namespace Sosage
{

class Content
{
public:
  
  typedef std::unordered_map<std::string, Component::Handle> Entity;

  
private:
  
  typedef std::unordered_map<std::string, Entity> Map;

  Lockable<Map> m_data;

public:

  Content ();

  void lock_components() { m_data.lock(); }
  void unlock_components() { m_data.unlock(); }

  template <typename T>
  void set (const std::string& entity, const std::string& component,
            const std::shared_ptr<T>& t)
  {
    lock_components();
    m_data[entity][component] = t;
    unlock_components();
  }
  
  template <typename T, typename ... Args>
  void set (const std::string& entity, const std::string& component,
            Args ... args)
  {
    std::shared_ptr<T> new_component = std::make_shared<T>(args...);
    set (entity, component, new_component);
  }

  Map::const_iterator begin() { return m_data.begin(); }
  Map::const_iterator end() { return m_data.end(); }

  void remove (const std::string& entity, const std::string& component)
  {
    lock_components();
    Map::iterator iter = m_data.find(entity);
    if (iter == m_data.end())
      error ("Entity " + entity + " doesn't exist");

    Entity::iterator iter2 = iter->second.find(component);
    if (iter2 == iter->second.end())
      error ("Component " + entity + ":" + component + " doesn't exist");

    iter->second.erase(iter2);
    if (iter->second.empty())
      m_data.erase(iter);
    unlock_components();
  }

  template <typename T>
  std::shared_ptr<T> get (const std::string& entity,
                          const std::string& component)
  {
    lock_components();
    Map::iterator iter = m_data.find(entity);
    if (iter == m_data.end())
    {
      unlock_components();
      return std::shared_ptr<T>();
    }

    Entity::iterator iter2 = iter->second.find(component);
    if (iter2 == iter->second.end())
    {
      unlock_components();
      return std::shared_ptr<T>();
    }

    std::shared_ptr<T> out = Component::component_cast<T>(iter2->second);
    unlock_components();

    return out;
  }
  
  

};

} // namespace Sosage

#endif // SOSAGE_CONTENT_H
