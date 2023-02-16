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

#include <Sosage/Component/Base.h>
#include <Sosage/Component/Handle_set.h>
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
  ~Content();
  void clear();
  void clear (const std::function<bool(Component::Handle)>& filter);
  std::size_t size() const;
  Component::Component_map::const_iterator begin() const;
  Component::Component_map::const_iterator end() const;
  Component::Handle_set components (const std::string& s);
  bool remove (const std::string& entity, const std::string& component, bool optional = false);
  bool remove (Component::Handle handle, bool optional = false);

  template <typename T>
  void set (const std::shared_ptr<T>& t)
  {
    count_set_ptr();
    Component::Handle_map& hmap = handle_map(t->component());
    Component::Handle_map::iterator iter = hmap.find(t->id());
    if (iter != hmap.end())
      hmap.erase(iter);
    hmap.insert(std::make_pair (t->id(), t));
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
  std::shared_ptr<T> get_or_set (const std::string& entity, const std::string& component, Args&& ... args)
  {
    count_set_args();
    if (auto out = request<T>(entity, component))
      return out;
    return set<T> (entity, component, args...);
  }

  template <typename T>
  std::shared_ptr<T> request (const std::string& entity, const std::string& component)
  {
    count_access(entity, component);
    count_request();
    Component::Handle_map& hmap = handle_map(component);
    Component::Handle_map::iterator iter = hmap.find(Component::Id(entity, component));
    if (iter == hmap.end())
      return std::shared_ptr<T>();

    std::shared_ptr<T> out = Component::cast<T>(iter->second);
    return out;
  }

  template <typename T>
  typename T::const_reference value (const std::string& entity, const std::string& component)
  {
    return get<T>(entity, component)->value();
  }

  template <typename T>
  typename T::value_type value (const std::string& entity, const std::string& component,
                                const typename T::value_type& default_value)
  {
    if (auto t = request<T>(entity, component))
      return t->value();
    return default_value;
  }

  template <typename T>
  std::shared_ptr<T> get (const std::string& entity, const std::string& component)
  {
    count_get();
    std::shared_ptr<T> out = request<T>(entity, component);
    check (out != std::shared_ptr<T>(), "Cannot find " + entity + ":" + component);
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

  void emit (const std::string& entity, const std::string& component);
  bool receive (const std::string& signal, const std::string& component);
  bool signal (const std::string& entity, const std::string& component);

private:

  Component::Handle_map& handle_map (const std::string& s);

  void count_set_ptr();
  void count_set_args();
  void count_request();
  void count_get();

#ifdef SOSAGE_PROFILE
  std::unordered_map<std::string, std::size_t> m_access_count;
  void count_access (const std::string& entity, const std::string& component);
  void display_access();
#else
  void count_access (const std::string&, const std::string&);
  void display_access ();
#endif
};

} // namespace Sosage

#endif // SOSAGE_CONTENT_H
