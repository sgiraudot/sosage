/*
  [include/Sosage/System/Handle.h]
  Virtual basis for all systems.

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

#ifndef SOSAGE_SYSTEM_HANDLE_H
#define SOSAGE_SYSTEM_HANDLE_H

#include <Sosage/Component/Locale.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Content.h>

#include <memory>
#include <string>
#include <unordered_set>

namespace Sosage::System
{

class Base
{
protected:

  Content& m_content;

public:

  Base (Content& content) : m_content (content) { }
  virtual ~Base() { }
  virtual void init() { }
  virtual void run() = 0;

#ifdef SOSAGE_PROFILE
  Time::Unit m_start;
  void start_timer ()
  {
    m_start = Time::now();
  }
  void stop_timer (const std::string& id)
  {
    set<Component::Int>(id + ":time", Time::now() - m_start);
  }

#else
  void start_timer () { }
  void stop_timer (const char*) { }
#endif

  template <typename T>
  std::shared_ptr<T> get (const std::string& key) { return m_content.get<T>(key); }
  template <typename T>
  std::shared_ptr<T> get (const Fast_access_component& fac) { return m_content.get<T>(fac); }
  template <typename T>
  std::shared_ptr<T> request (const std::string& key) { return m_content.request<T>(key); }
  template <typename T>
  typename T::const_reference value (const Fast_access_component& fac) { return m_content.value<T>(fac); }
  template <typename T>
  typename T::const_reference value (const std::string& key) { return m_content.value<T>(key); }
  template <typename T>
  typename T::value_type value (const std::string& key, const typename T::value_type& default_value)
  { return m_content.value<T>(key, default_value); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> set (Args&& ... args) { return m_content.set<T>(std::forward<Args>(args)...); }
  template <typename T>
  void set (const std::shared_ptr<T>& t) { return m_content.set<T>(t); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> get_or_set (const std::string& id, Args&& ... args)
  { return m_content.get_or_set<T>(id, std::forward<Args>(args)...); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> set_fac (const Fast_access_component& fac, Args&& ... args)
  { return m_content.set_fac<T>(fac, std::forward<Args>(args)...); }
  void remove (const std::string& key, bool optional = false) { m_content.remove(key, optional); }
  void emit (const std::string& signal) { m_content.emit (signal); }
  bool receive (const std::string& signal) { return m_content.receive(signal); }
  Component::Status_handle status() { return get<Component::Status>(GAME__STATUS); }

  const std::string& locale (const std::string& line)
  {
    if (auto l = request<Component::Locale>("Game:locale"))
      return l->get(line);
    // else
    return line;
  }

  const std::string& locale_get (const std::string& id)
  {
    return locale (value<Component::String>(id));
  }
};

using Handle = std::shared_ptr<Base>;

template <typename T, typename ... Args>
std::shared_ptr<T> make_handle (Args& ... args)
{
  return std::make_shared<T>(args...);
}


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_H
