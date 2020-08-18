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

  Component::Handle_set m_data;
  std::array<Component::Handle, NUMBER_OF_KEYS> m_fast_access_components;

public:

  Content ();

  Content (const Content&) = delete;

  ~Content()
  {
    display_access();
  }

  void clear() { m_data.clear(); }
  std::size_t size() const { return m_data.size(); }
  Component::Handle_set::const_iterator begin() const { return m_data.begin(); }
  Component::Handle_set::const_iterator end() const { return m_data.end(); }
  void remove (const std::string& key, bool optional = false);

  void clear (const std::function<bool(Component::Handle)>& filter)
  {
    Component::Handle_set new_set;
    for (Component::Handle c : m_data)
      if (!filter(c))
        new_set.insert(c);
    m_data.swap (new_set);
  }

  template <typename T>
  void set (const std::shared_ptr<T>& t)
  {
    count_set_ptr();
    Component::Handle_set::iterator iter = m_data.find(t);
    if (iter != m_data.end())
      m_data.erase(iter);
    m_data.insert(t);
  }

  template <typename T, typename ... Args>
  std::shared_ptr<T> set (Args&& ... args)
  {
    count_set_args();
    std::shared_ptr<T> new_component = std::make_shared<T>(args...);
    set (new_component);
    return new_component;
  }

  template <typename T>
  std::shared_ptr<T> request (const std::string& key)
  {
    count_access(key);
    count_request();
    Component::Handle_set::iterator iter = m_data.find(std::make_shared<Component::Base>(key));
    if (iter == m_data.end())
      return std::shared_ptr<T>();

    std::shared_ptr<T> out = Component::cast<T>(*iter);
    return out;
  }

  template <typename T>
  std::shared_ptr<T> get (const std::string& key)
  {
    count_get();
    std::shared_ptr<T> out = request<T>(key);
#ifdef SOSAGE_DEBUG
    if (out == std::shared_ptr<T>())
    {
      std::cerr << "Candidate are:" << std::endl;
      std::string entity (key.begin(), key.begin() + key.find_first_of(':'));
      for (Component::Handle h : m_data)
        if (h->entity() == entity)
          std::cerr << " * " << h->id() << std::endl;
    }
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
    std::sort (sorted.begin(), sorted.end(),
               [](const auto& a, const auto& b) -> bool
               {
                 return a.second > b.second;
               });
    std::cerr << "[Profiling component access count (10 first)]" << std::endl;
    for (std::size_t i = 0; i < 10; ++ i)
      std::cerr << " * " << sorted[i].first << " (accessed "
                << sorted[i].second << " times)" << std::endl;
  }
#else
  inline void count_access (const std::string&) { }
  inline void display_access () { }
#endif
};

} // namespace Sosage

#endif // SOSAGE_CONTENT_H
