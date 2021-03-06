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

  template <typename T>
  std::shared_ptr<T> get (const std::string& key) { return m_content.get<T>(key); }
  template <typename T>
  std::shared_ptr<T> get (const Fast_access_component& fac) { return m_content.get<T>(fac); }
  template <typename T>
  std::shared_ptr<T> request (const std::string& key) { return m_content.request<T>(key); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> set (Args&& ... args) { return m_content.set<T>(std::forward<Args>(args)...); }
  template <typename T>
  void set (const std::shared_ptr<T>& t) { return m_content.set<T>(t); }
  template <typename T, typename ... Args>
  std::shared_ptr<T> set_fac (const Fast_access_component& fac, Args&& ... args)
  { return m_content.set_fac<T>(fac, std::forward<Args>(args)...); }
  void remove (const std::string& key, bool optional = false) { m_content.remove(key, optional); }
  void emit (const std::string& signal) { m_content.emit (signal); }
  bool receive (const std::string& signal) { return m_content.receive(signal); }

};

using Handle = std::shared_ptr<Base>;

template <typename T, typename ... Args>
std::shared_ptr<T> make_handle (Args& ... args)
{
  return std::make_shared<T>(args...);
}


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_H
