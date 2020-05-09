/*
  [include/Sosage/Component/Event.h]
  Used to communicate string-based events between systems.

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

#ifndef SOSAGE_COMPONENT_EVENT_H
#define SOSAGE_COMPONENT_EVENT_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class Event : public Base
{
public:

  Event (const std::string& id) : Base(id) { }
  
};

typedef std::shared_ptr<Event> Event_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONDITION_H
