/*
  [include/Sosage/Component/Text.h]
  A string holding text to be displayed.

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

#ifndef SOSAGE_COMPONENT_TEXT_H
#define SOSAGE_COMPONENT_TEXT_H

#include <Sosage/Component/Handle.h>

namespace Sosage::Component
{

class Text : public Base
{
private:
  std::string m_value;
  
public:

  Text (const std::string& id, const std::string& value);
  const std::string& value() const { return m_value; }
};

typedef std::shared_ptr<Text> Text_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_TEXT_H
