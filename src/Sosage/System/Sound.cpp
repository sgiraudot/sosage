/*
  [src/Sosage/System/Sound.cpp]
  Sounds and musics.

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

#include <Sosage/Component/Code.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/Config/platform.h>
#include <Sosage/System/Sound.h>

namespace Sosage::System
{

Sound::Sound (Content& content)
  : m_content (content), m_core()
{

}

void Sound::run()
{
  auto music = m_content.request<Component::Music>("game:music");

  auto start = m_content.request<Component::Event>("music:start");
  if (start)
  {
    m_core.start_music (music->core());
    music->on() = true;
    m_content.remove ("music:start");
  }
  
  bool paused = m_content.get<Component::Boolean>("game:paused")->value();
  if (paused && music->on())
  {
    m_core.pause_music (music->core());
    music->on() = false;
  }
  else if (!paused && !music->on())
  {
    m_core.resume_music (music->core());
    music->on() = true;
  }

  
  if (auto clicked = m_content.request<Component::Event>("game:verb_clicked"))
  {
    m_core.play_sound (m_content.get<Component::Sound>("click:sound")->core());
    m_content.remove ("game:verb_clicked");
  }
  
  if (auto failure = m_content.request<Component::Event>("code:play_failure"))
  {
    m_core.play_sound
      (m_content.get<Component::Sound>
       (m_content.get<Component::Code>("game:code")->entity() +"_button:sound")->core());
    m_content.remove ("code:play_failure");
  }
  else if (auto success = m_content.request<Component::Event>("code:play_success"))
  {
    m_core.play_sound
      (m_content.get<Component::Sound>
       (m_content.get<Component::Code>("game:code")->entity() +"_success:sound")->core());
    m_content.remove ("code:play_success");
  }
  else if (auto button = m_content.request<Component::Event>("code:play_click"))
  {
    m_core.play_sound
      (m_content.get<Component::Sound>
       (m_content.get<Component::Code>("game:code")->entity() +"_failure:sound")->core());
    m_content.remove ("code:play_click");
  }

}

} // namespace Sosage::System
