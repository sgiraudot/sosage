/*
  [include/Sosage/Component/Font.h]
  Font container for text generation.

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

#ifndef SOSAGE_COMPONENT_FONT_H
#define SOSAGE_COMPONENT_FONT_H

#include <Sosage/Component/Base.h>
#include <Sosage/Core/Graphic.h>

namespace Sosage::Component
{

class Font : public Base
{
private:
  Core::Graphic::Font m_core;
  
public:

  Font (const std::string& entity, const std::string& component, const std::string& file_name, int size);
  virtual ~Font();
  const Core::Graphic::Font& core() const;
};

using Font_handle = std::shared_ptr<Font>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_FONT_H
