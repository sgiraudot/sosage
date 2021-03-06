/*
  [include/Sosage/Component/Music.h]
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

#ifndef SOSAGE_COMPONENT_MUSIC_H
#define SOSAGE_COMPONENT_MUSIC_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Core/Sound.h>

namespace Sosage::Component
{

class Music : public Base
{
private:
  Core::Sound::Music m_core;
  bool m_on;
  
public:

  Music (const std::string& id, const std::string& file_name);
  virtual ~Music();
  virtual std::string str() const;
  const Core::Sound::Music& core() const { return m_core; }
  const bool& on() const { return m_on; }
  bool& on() { return m_on; }

};

using Music_handle = std::shared_ptr<Music>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_MUSIC_H
