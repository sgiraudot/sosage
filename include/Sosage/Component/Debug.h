/*
  [include/Sosage/Component/Debug.h]
  Debug information on screen (FPS, etc.).

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

#ifndef SOSAGE_COMPONENT_DEBUG_H
#define SOSAGE_COMPONENT_DEBUG_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Content.h>
#include <Sosage/Utils/time.h>

namespace Sosage::Component
{

class Debug : public Boolean
{
private:

  const Content& m_content;
  const Clock& m_clock;
  
public:

  Debug (const std::string& id, const Content& content, const Clock& clock);
  virtual ~Debug();
  std::string debug_str() const;
};

typedef std::shared_ptr<Debug> Debug_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_DEBUG_H
