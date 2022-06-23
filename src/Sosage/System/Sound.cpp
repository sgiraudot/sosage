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
  SOSAGE_TIMER_START(System_Sound__run);
  SOSAGE_UPDATE_DBG_LOCATION("Sound::run()");

  auto music = request<C::Music>("Game", "music");

  double volume = value<C::Int>("Music", "volume") / 10.;
  // Low level when playing, higher on cutscenes
  if (!status()->is(CUTSCENE))
    volume *= 0.25;

  if (receive("Music", "stop"))
  {
    m_core.stop_music();
    if (music)
      music->on() = false;
  }

  if (receive("Music", "start"))
  {
    debug << value<C::Double>(CLOCK__TIME) << ": music start" << std::endl;
    check (music, "No music to start");
    m_core.start_music (music->core(), volume);
    music->on() = true;
  }

  if (auto fade = request<C::Tuple<double, double, bool>>("Music", "fade"))
  {
    check (music, "No music to fade");
    double current_time = value<C::Double> (CLOCK__TIME);
    m_core.stop_music();
    m_core.set_volume (volume);
    m_core.fade(music->core(), fade->get<1>() - current_time, fade->get<2>());
    music->on() = true;
    remove("Music", "fade");
  }

  if (receive("Music", "volume_changed"))
    m_core.set_volume (volume);

  if (music)
  {
    bool paused = (status()->is (PAUSED));

    if (paused && music->on())
    {
      if (status()->was (CUTSCENE))
        m_core.pause_music (music->core());
      else
        m_core.set_volume(0.15 * volume);
      music->on() = false;
    }
    else if (!paused && !music->on())
    {
      if (status()->is (CUTSCENE))
        m_core.resume_music(music->core());
      else
        m_core.set_volume (volume);
      music->on() = true;
    }
  }

  if (receive ("code", "play_failure"))
    m_core.play_sound
      (get<C::Sound>
       (get<C::Code>("Game", "code")->entity() +"_failure", "sound")->core(),
       value<C::Int>("Sounds", "volume") / 10.);
  else if (receive ("code", "play_success"))
    m_core.play_sound
      (get<C::Sound>
       (get<C::Code>("Game", "code")->entity() +"_success", "sound")->core(),
       value<C::Int>("Sounds", "volume") / 10.);
  else if (receive ("code", "play_click"))
    m_core.play_sound
      (get<C::Sound>
       (get<C::Code>("Game", "code")->entity() +"_button", "sound")->core(),
       value<C::Int>("Sounds", "volume") / 10.);

  std::vector<C::Handle> to_remove;
  for (C::Handle ev : components("play_sound"))
  {
    m_core.play_sound (get<C::Sound> (ev->entity() , "sound")->core(),
                       value<C::Int>("Sounds", "volume") / 10.,
                       value<C::Double>(ev->entity(), "panning", 0.5));
    to_remove.push_back (ev);
  }

  for (C::Handle c : to_remove)
    remove (c);

  SOSAGE_TIMER_STOP(System_Sound__run);
}

} // namespace Sosage::System
