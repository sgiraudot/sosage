/*
  [src/Sosage/Component/Sound.cpp]
  Sound effect that can be triggered.

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

#include <Sosage/Component/Sound.h>

namespace Sosage::Component
{

Sound::Sound (const std::string& id, const std::string& file_name)
  : Base(id)
{
  m_core = Core::Sound::load_sound (file_name);
}

Sound::~Sound()
{
  Core::Sound::delete_sound(m_core);
}

std::string Sound::str() const
{
  return this->id();
}

const Core::Sound::Sound& Sound::core() const
{
  return m_core;
}

} // namespace Sosage::Component
