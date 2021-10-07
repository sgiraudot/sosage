/*
  [include/Sosage/Content.h]
  Stores and handles access to all variable game content (components).

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#ifndef SOSAGE_CONTENT_H
#define SOSAGE_CONTENT_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Component/Signal.h>
#include <Sosage/Component/cast.h>
#include <Sosage/Utils/enum.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Utils/profiling.h>

#include <array>
#include <functional>

namespace Sosage
{

class Content
{
private:

  Component::Component_map m_data;
  std::array<Component::Handle, NUMBER_OF_KEYS> m_fast_access_components;
  std::unordered_map<std::string, std::size_t> m_map_component;

public:

  Content ();

  Content (const Content&) = delete;

  ~Content()
  {
    display_access();
  }

  void clear()
  {
    for (std::size_t i = 0; i < NUMBER_OF_KEYS; ++ i)
      m_fast_access_components[i] = nullptr;
    m_data.clear();

  }
  std::size_t size() const { return m_data.size(); }
  Component::Component_map::const_iterator begin() const { return m_data.begin(); }
  Component::Component_map::const_iterator end() const { return m_data.end(); }
  Component::Handle_set& components (const std::string& s)
  {
    auto iter = m_map_component.find(s);
    if (iter == m_map_component.end())
        return m_data[0];
    return m_data[iter->second];
  }

  void remove (const std::string& key, bool optional = false);

  void clear (const std::function<bool(Component::Handle)>& filter)
  {
    for (auto& hset : m_data)
    {
      Component::Handle_set& old_set = hset;
      Component::Handle_set new_set;
      for (Component::Handle c : old_set)
        if (!filter(c))
          new_set.insert(c);
      old_set.swap (new_set);
    }
  }

  template <typename T>
  void set (const std::shared_ptr<T>& t)
  {
    count_set_ptr();
    Component::Handle_set& hset = components(t->component());
    Component::Handle_set::iterator iter = hset.find(t);
    if (iter != hset.end())
      hset.erase(iter);
    hset.insert(t);
  }

  template <typename T, typename ... Args>
  std::shared_ptr<T> set (Args&& ... args)
  {
    count_set_args();
    std::shared_ptr<T> new_component = std::make_shared<T>(args...);
    set (new_component);
    return new_component;
  }

  template <typename T, typename ... Args>
  std::shared_ptr<T> get_or_set (const std::string& id, Args&& ... args)
  {
    count_set_args();
    if (auto out = request<T>(id))
      return out;
    return set<T> (id, args...);
  }

  template <typename T>
  std::shared_ptr<T> request (const std::string& key)
  {
    count_access(key);
    count_request();
    auto cmp = std::make_shared<Component::Base>(key);
    Component::Handle_set& hset = components(cmp->component());
    Component::Handle_set::iterator iter = hset.find(std::make_shared<Component::Base>(key));
    if (iter == hset.end())
      return std::shared_ptr<T>();

    std::shared_ptr<T> out = Component::cast<T>(*iter);
    return out;
  }

  template <typename T>
  typename T::const_reference value (const std::string& key)
  {
    return get<T>(key)->value();
  }

  template <typename T>
  typename T::value_type value (const std::string& key,
                                const typename T::value_type& default_value)
  {
    if (auto t = request<T>(key))
      return t->value();
    return default_value;
  }

  template <typename T>
  std::shared_ptr<T> get (const std::string& key)
  {
    count_get();
    std::shared_ptr<T> out = request<T>(key);
#ifdef SOSAGE_DEBUG
#if 0
    if (out == std::shared_ptr<T>())
    {
      debug << "Candidate are:" << std::endl;
      std::string entity (key.begin(), key.begin() + key.find_first_of(':'));
      for (Component::Handle h : m_data)
        if (h->entity() == entity)
          debug << " * " << h->id() << std::endl;
    }
#endif
#endif
    check (out != std::shared_ptr<T>(), "Cannot find " + key);
    return out;
  }

  // Fast access version
  template <typename T, typename ... Args>
  std::shared_ptr<T> set_fac (const Fast_access_component& fac, Args&& ... args)
  {
    std::shared_ptr<T> new_component = set<T>(args...);
    m_fast_access_components[std::size_t(fac)] = new_component;
    return new_component;
  }

  template <typename T>
  std::shared_ptr<T> get (const Fast_access_component& fac)
  {
    return Component::cast<T>(m_fast_access_components[std::size_t(fac)]);
  }

  template <typename T>
  typename T::const_reference value (const Fast_access_component& fac)
  {
    return Component::cast<T>(m_fast_access_components[std::size_t(fac)])->value();
  }

  void emit (const std::string& signal)
  {
    set<Component::Signal>(signal);
  }

  bool receive (const std::string& signal)
  {
    count_access(signal);
    count_request();

    auto cmp = std::make_shared<Component::Base>(signal);
    Component::Handle_set& hset = components(cmp->component());

    Component::Handle_set::iterator iter
        = hset.find(std::make_shared<Component::Base>(signal));
    if (iter == hset.end())
      return false;
    if (!Component::cast<Component::Signal>(*iter))
      return false;
    hset.erase (iter);
    return true;
  }

private:

  inline void count_set_ptr() { SOSAGE_COUNT (Content__set_ptr); }
  inline void count_set_args() { SOSAGE_COUNT (Content__set_args); }
  inline void count_request() { SOSAGE_COUNT (Content__request); }
  inline void count_get() { SOSAGE_COUNT (Content__get); }

#ifdef SOSAGE_PROFILE
  std::unordered_map<std::string, std::size_t> m_access_count;
  inline void count_access (const std::string& k)
  {
    auto inserted = m_access_count.insert (std::make_pair (k, 1));
    if (!inserted.second)
      inserted.first->second ++;
  }
  inline void display_access()
  {
    std::vector<std::pair<std::string, std::size_t> > sorted;
    sorted.reserve (m_access_count.size());
    std::copy (m_access_count.begin(), m_access_count.end(),
               std::back_inserter (sorted));
    auto end = std::partition
               (sorted.begin(), sorted.end(),
                [](const auto& p) -> bool { return isupper(p.first[0]); });
    std::sort (sorted.begin(), end,
               [](const auto& a, const auto& b) -> bool
               {
                 return a.second > b.second;
               });
    debug << "[Profiling system component access count (25 first)]" << std::endl;
    for (std::size_t i = 0; i < 25; ++ i)
      debug << " * " << sorted[i].first << " (accessed "
            << sorted[i].second << " times)" << std::endl;
  }
#else
  inline void count_access (const std::string&) { }
  inline void display_access () { }
#endif
};

} // namespace Sosage

#endif // SOSAGE_CONTENT_H
