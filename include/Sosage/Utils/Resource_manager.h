/*
  [include/Sosage/Utils/Resource_manager.h]
  Avoid loading the same file twice.

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

#ifndef SOSAGE_UTILS_RESOURCE_MANAGER_H
#define SOSAGE_UTILS_RESOURCE_MANAGER_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Content.h>
#include <Sosage/Utils/time.h>

#include <functional>

namespace Sosage
{

template <typename Resource>
class Resource_manager
{
public:

  using Resource_handle = std::shared_ptr<Resource>;
  using Data = std::unordered_map<std::string, Resource_handle> ;
  
private:

  Data m_data;
  std::function<void(Resource*)> m_deleter;
  
public:

  Resource_manager(const std::function<void(Resource*)>& deleter
                   = [](Resource* r) { delete r; })
    : m_deleter(deleter)
  { }

  void clear() { m_data.clear(); }

  Resource_handle make_single_base (const std::function<Resource*()>& f)
  {
    return Resource_handle(f(), m_deleter);
  }

  template <typename F, typename ... Args>
  Resource_handle make_single (F&& f, Args&& ... args)
  {
    std::function<Resource*()> func = std::bind (f, args...);
    return make_single_base (func);
  }

  template <typename F, typename ... Args>
  Resource_handle make_mapped (const std::string& key, F&& f, Args&& ... args)
  {
    typename Data::iterator iter;
    bool inserted;
    std::tie (iter, inserted) = m_data.insert (std::make_pair (key, Resource_handle()));
    if (inserted)
    {
      debug ("Loading ", key);
      iter->second = make_single(std::forward<F>(f), std::forward<Args>(args)...);
    }
    else
      debug ("Not reloading ", key);
    return iter->second;
  }
};

} // namespace Sosage::Component

#endif // SOSAGE_UTILS_RESOURCE_MANAGER_H
