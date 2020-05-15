/*
  [src/Sosage/Component/Font.cpp]
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

#include <Sosage/Component/Font.h>
#include <Sosage/Config/config.h>

namespace Sosage::Component
{

Font::Font (const std::string& id, const std::string& file_name, int size)
  : Base(id)
{
  m_core = Core::Graphic::load_font (file_name, size);
}

Font::~Font()
{
  Core::Graphic::delete_font(m_core);
}

} // namespace Sosage::Component
