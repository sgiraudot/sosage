/*
  [src/Sosage/Component/Music.cpp]
  Background music played in loop.

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

#include <Sosage/Component/Music.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Music::Music (const std::string& id, const std::string& file_name)
  : Base(id), m_on(false)
{
  m_core = Core::Sound::load_music (file_name);
}

Music::~Music()
{
  Core::Sound::delete_music(m_core);
}

std::string Music::str() const
{
  return this->id();
}



} // namespace Sosage::Component
