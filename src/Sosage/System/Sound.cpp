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
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Config/platform.h>
#include <Sosage/System/Sound.h>

namespace Sosage::System
{

namespace C = Component;

Sound::Sound (Content& content)
  : Base (content), m_core()
{

}

void Sound::run()
{
  auto status = m_content.get<C::Status>(GAME__STATUS);
  if (status->value() == LOADING)
    return;

  auto music = m_content.request<C::Music>("game:music");

  if (receive("music:start"))
  {
    check (music, "No music to start");
    m_core.start_music (music->core());
    music->on() = true;
    remove("music:start");
  }

  if (receive("music:fade"))
  {
    check (music, "No music to fade");
    m_core.stop_music();
    auto fadein = request<C::Boolean>("fade:in");
    double current_time = get<C::Double> (CLOCK__FRAME_TIME)->value();
    double end_time = get<C::Double>("fade:end")->value();
    m_core.fade(music->core(), end_time - current_time, fadein->value());
    remove ("music:fade");
  }

  if (receive("music:stop"))
  {
    m_core.stop_music();
    remove ("music:stop");
  }

  if (music)
  {
    bool paused = (status->value() == PAUSED);
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
  }

  if (auto clicked = receive ("game:verb_clicked"))
  {
    m_core.play_sound (m_content.get<C::Sound>("click:sound")->core());
    m_content.remove ("game:verb_clicked");
  }

  if (auto failure = receive ("code:play_failure"))
  {
    m_core.play_sound
      (m_content.get<C::Sound>
       (m_content.get<C::Code>("game:code")->entity() +"_button:sound")->core());
    m_content.remove ("code:play_failure");
  }
  else if (auto success = receive ("code:play_success"))
  {
    m_core.play_sound
      (m_content.get<C::Sound>
       (m_content.get<C::Code>("game:code")->entity() +"_success:sound")->core());
    m_content.remove ("code:play_success");
  }
  else if (auto button = receive ("code:play_click"))
  {
    m_core.play_sound
      (m_content.get<C::Sound>
       (m_content.get<C::Code>("game:code")->entity() +"_failure:sound")->core());
    m_content.remove ("code:play_click");
  }

  std::vector<std::string> to_remove;
  for (C::Handle h : m_content)
    if (auto ev = C::cast<C::Signal>(h))
      if (ev->entity() == "play_sound")
      {
        m_core.play_sound (m_content.get<C::Sound> (ev->component() + ":sound")->core());
        to_remove.push_back (ev->id());
      }

  for (const std::string& id : to_remove)
    remove (id);
}

} // namespace Sosage::System
