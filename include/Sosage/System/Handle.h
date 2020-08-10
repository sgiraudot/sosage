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

#include <memory>
#include <string>
#include <unordered_set>

namespace Sosage::System
{

class Base
{
public:

  virtual ~Base() { }
  virtual void init() { }
  virtual void run() = 0;
};

typedef std::shared_ptr<Base> Handle;

template <typename T, typename ... Args>
std::shared_ptr<T> make_handle (Args& ... args)
{
  return std::make_shared<T>(args...);
}


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_HANDLE_H
