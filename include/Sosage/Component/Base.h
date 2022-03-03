/*
  [include/Sosage/Component/Base.h]
  Virtual basis for all components.

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

#ifndef SOSAGE_COMPONENT_BASE_H
#define SOSAGE_COMPONENT_BASE_H

#include <memory>
#include <string>

#define STR_NAME(x) virtual std::string str_name() const { return x; }
#define STR_VALUE(x) virtual std::string str_value() const { return x; }
#define STR_SUB(x) virtual std::string str_sub(const std::size_t& indent) const { x }

namespace Sosage::Component
{

using Id = std::pair<std::string, std::string>;

class Base
{
  Id m_id;
  bool m_altered;

public:
  Base (const std::string& entity, const std::string& component);
  virtual ~Base();
  bool is_system() const;
  void mark_as_altered();
  void mark_as_unaltered();
  bool was_altered() const;
  const Id& id() const;
  const std::string& entity() const;
  std::string character_entity() const;
  std::string target_entity() const;
  const std::string& component() const;
  std::string str() const;

  STR_NAME("Base");
  STR_VALUE("");
  virtual std::string str_sub(const std::size_t&) const { return ""; }
};

using Handle = std::shared_ptr<Base>;

std::string component_str(Handle handle, const std::size_t& indent, const std::string& prefix = "");

template <typename T>
class Value : public Base
{
public:
  Value(const std::string& entity, const std::string& component)
    : Base(entity, component) { }
  virtual T value() const = 0;
};

template <typename T>
using Value_handle = std::shared_ptr<Value<T> >;

template <typename T, typename ... Args>
std::shared_ptr<T> make_handle (Args ... args)
{
  return std::make_shared<T>(args...);
}

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_BASE_H
