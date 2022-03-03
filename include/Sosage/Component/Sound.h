/*
  [include/Sosage/Component/Sound.h]
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

#ifndef SOSAGE_COMPONENT_SOUND_H
#define SOSAGE_COMPONENT_SOUND_H

#include <Sosage/Component/Base.h>
#include <Sosage/Core/Sound.h>

namespace Sosage::Component
{

class Sound : public Base
{
private:
  Core::Sound::Sound m_core;
  
public:

  Sound (const std::string& entity, const std::string& component, const std::string& file_name);
  virtual ~Sound();
  const Core::Sound::Sound& core() const;

  STR_NAME("Sound");
};

using Sound_handle = std::shared_ptr<Sound>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_SOUND_H
