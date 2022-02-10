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

  class Handles
  {
    using Base = Component::Handle_set;
    Base& m_base;

  public:
    class iterator
    {
      using Base = Component::Handle_set::iterator;
      Base m_base;

    public:

      using iterator_category = std::forward_iterator_tag;
      using difference_type   = std::ptrdiff_t;
      using value_type        = Component::Handle;
      using pointer           = value_type*;
      using reference         = value_type&;

      iterator (Base base = Base()) : m_base(base) { }

      reference operator*() const { return m_base->second; }
      pointer operator->() { return &m_base->second; }
      iterator& operator++() { m_base++; return *this; }
      iterator operator++(int) { iterator tmp = *this; ++(*this); return tmp; }
      friend bool operator== (const iterator& a, const iterator& b) { return a.m_base == b.m_base; };
      friend bool operator!= (const iterator& a, const iterator& b) { return a.m_base != b.m_base; };
    };

    using const_iterator = iterator;

    Handles (Base& base) : m_base (base) { }

    iterator begin() { return iterator(m_base.begin()); }
    iterator end() { return iterator(m_base.end()); }
    const_iterator begin() const { return iterator(m_base.begin()); }
    const_iterator end() const { return iterator(m_base.end()); }
  };

  Content ();
  Content (const Content&) = delete;
  ~Content();
  void clear();
  void clear (const std::function<bool(Component::Handle)>& filter);
  std::size_t size() const;
  Component::Component_map::const_iterator begin() const;
  Component::Component_map::const_iterator end() const;
  Handles components (const std::string& s);
  void remove (const std::string& key, bool optional = false);

  template <typename T>
  void set (const std::shared_ptr<T>& t)
  {
    count_set_ptr();
    Component::Handle_set& hset = handle_set(t->component());
    Component::Handle_set::iterator iter = hset.find(t->id());
    if (iter != hset.end())
      hset.erase(iter);
    hset.insert(std::make_pair (t->id(), t));
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
    Component::Handle_set& hset = handle_set(component(key));
    Component::Handle_set::iterator iter = hset.find(key);
    if (iter == hset.end())
      return std::shared_ptr<T>();

    std::shared_ptr<T> out = Component::cast<T>(iter->second);
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

  void emit (const std::string& signal);
  bool receive (const std::string& signal);

private:

  std::string component (const std::string& id) const
  {
    std::size_t pos = id.find_first_of(':');
    return std::string (id.begin() + pos + 1, id.end());
  }

  Component::Handle_set& handle_set (const std::string& s);

  void count_set_ptr();
  void count_set_args();
  void count_request();
  void count_get();

#ifdef SOSAGE_PROFILE
  std::unordered_map<std::string, std::size_t> m_access_count;
  void count_access (const std::string& k);
  void display_access();
#else
  void count_access (const std::string&);
  void display_access ();
#endif
};

} // namespace Sosage

#endif // SOSAGE_CONTENT_H
