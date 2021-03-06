/*
  [include/Sosage/Component/Handle.h]
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

#ifndef SOSAGE_COMPONENT_HANDLE_H
#define SOSAGE_COMPONENT_HANDLE_H

#include <Sosage/Utils/error.h>

#include <memory>
#include <string>
#include <unordered_set>

namespace Sosage::Component
{

class Base
{
  std::string m_id; // entity:component
  
public:
  Base(const std::string& id) : m_id (id) { }
  virtual ~Base() { }

  bool is_system() const { return isupper(m_id[0]); }

  const std::string& id() const { return m_id; }

  std::string entity() const
  {
    return std::string (m_id.begin(), m_id.begin() + m_id.find_first_of(':'));
  }

  // Special handling of entity for characters
  std::string character_entity() const
  {
    for (const std::string& postfix : { "_body", "_head", "_idle", "_mouth", "_walking" })
    {
      std::size_t pos = m_id.find(postfix);
      if (pos != std::string::npos)
        return std::string (m_id.begin(), m_id.begin() + pos);
    }
    return std::string (m_id.begin(), m_id.begin() + m_id.find_first_of(':'));
  }

  // Special handling of entity for binary actions
  std::string target_entity() const
  {
    for (const std::string& prefix : { ":use_" })
    {
      std::size_t pos = m_id.find(prefix);
      if (pos != std::string::npos)
        return std::string (m_id.begin() + pos + prefix.size(), m_id.end());
    }
    return std::string (m_id.begin(), m_id.begin() + m_id.find_first_of(':'));
  }

  std::string component() const
  {
    return std::string (m_id.begin() + m_id.find_last_of(':') + 1, m_id.end());
  }

  virtual std::string str() const { return m_id; }

  void set_id (const std::string& id) { m_id = id; }
};

using Handle = std::shared_ptr<Base>;

template <typename T>
class Value : public Base
{
public:
  Value(const std::string& id) : Base(id) { }
  virtual T value() const = 0;
};

template <typename T>
using Value_handle = std::shared_ptr<Value<T> >;

template <typename T, typename ... Args>
std::shared_ptr<T> make_handle (Args ... args)
{
  return std::make_shared<T>(args...);
}

struct Hash_ids
{
  std::size_t operator() (const Handle& h) const
  {
    return std::hash<std::string>()(h->id());
  }
};

struct Equal_ids
{
  bool operator() (const Handle& a,
                   const Handle& b) const
  {
    return (a->id() == b->id());
  }
};

using Handle_set = std::unordered_set<Handle, Hash_ids, Equal_ids>;
    
} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_H
