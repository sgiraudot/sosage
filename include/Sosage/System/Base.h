/*
  [include/Sosage/System/Base.h]
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

#ifndef SOSAGE_SYSTEM_BASE_H
#define SOSAGE_SYSTEM_BASE_H

#include <Sosage/Component/Handle_set.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Content.h>

#include <memory>
#include <string>

namespace Sosage::System
{

class Base
{
protected:

  Content& m_content;

#ifdef SOSAGE_PROFILE
  Time::Unit m_start;
#endif

public:

  Base (Content& content);
  virtual ~Base();
  virtual void init();
  virtual void run() = 0;

  Component::Handle_set components (const std::string& s);
  template <typename T>
  std::shared_ptr<T> get (const std::string& entity, const std::string& component)
  { return m_content.get<T>(entity, component); }
  template <typename T>
  std::shared_ptr<T> get (const Fast_access_component& fac) { return m_content.get<T>(fac); }
  template <typename T>
  std::shared_ptr<T> request (const std::string& entity, const std::string& component)
  { return m_content.request<T>(entity, component); }
  template <typename T>
  typename T::const_reference value (const Fast_access_component& fac) { return m_content.value<T>(fac); }
  template <typename T>
  typename T::const_reference value (const std::string& entity, const std::string& component)
  { return m_content.value<T>(entity, component); }
  template <typename T>
  typename T::value_type value (const std::string& entity, const std::string& component,
                                const typename T::value_type& default_value)
  { return m_content.value<T>(entity, component, default_value); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> set (Args&& ... args) { return m_content.set<T>(std::forward<Args>(args)...); }
  template <typename T>
  void set (const std::shared_ptr<T>& t) { return m_content.set<T>(t); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> get_or_set (const std::string& entity, const std::string& component, Args&& ... args)
  { return m_content.get_or_set<T>(entity, component, std::forward<Args>(args)...); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> set_fac (const Fast_access_component& fac, Args&& ... args)
  { return m_content.set_fac<T>(fac, std::forward<Args>(args)...); }
  bool remove (const std::string& entity, const std::string& component, bool optional = false);
  bool remove (Component::Handle handle, bool optional = false);
  void emit (const std::string& entity, const std::string& component);
  bool receive (const std::string& entity, const std::string& component);
  Component::Status_handle status();
  const std::string& locale (const std::string& line);
  const std::string& locale_get (const std::string& entity, const std::string& component);
};

using Handle = std::shared_ptr<Base>;

template <typename T, typename ... Args>
std::shared_ptr<T> make_handle (Args& ... args)
{
  return std::make_shared<T>(args...);
}

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_BASE_H
