/*
  [include/Sosage/Component/Console.h]
  Virtual GUI console to display debug info ingame.

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

#ifndef SOSAGE_COMPONENT_CONSOLE_H
#define SOSAGE_COMPONENT_CONSOLE_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Utils/time.h>

#include <sstream>

#define SOSAGE_USE_STDCERR
#if defined(SOSAGE_USE_STDCERR)
#  define DBG_CERR std::cerr
#else
#  include <Sosage/Component/Console.h>
#  define DBG_CERR m_content.get<Component::Console>("game:console")->content()
#endif


namespace Sosage::Component
{

class Console : public Boolean
{
private:

  std::stringstream m_content;
  
public:

  Console (const std::string& id);
  virtual ~Console() { }
  std::string console_str();
  
  std::stringstream& content() { return m_content; }
};

typedef std::shared_ptr<Console> Console_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CONSOLE_H
