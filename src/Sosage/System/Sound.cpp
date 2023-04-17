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
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Sound.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Config/config.h>
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
    volume *= 0.75;

  double time = value<C::Double>(CLOCK__TIME);

  bool volume_changed = false;
  if (music && receive("Music", "adjust_mix"))
  {
    const std::string& player = value<C::String>("Player", "name");
    if (music->adjust_mix(value<C::Position>(player + "_body", "position"), time))
      emit("Music", "adjust_mix"); // if source still fading, update next time
    volume_changed = true;
  }

  if (music && receive("Music", "stop"))
  {
    for (std::size_t i = 0; i < music->tracks(); ++ i)
      m_core.fade(music->core(i), i, Config::default_sound_fade_time, false);
    music->on() = false;
  }

  if (music && receive("Music", "start"))
  {
    check (music, "No music to start");
    m_core.set_music_channels(music->tracks());
    for (std::size_t i = 0; i < music->tracks(); ++ i)
      m_core.start_music (music->core(i), int(i), volume * music->mix(i));
    music->on() = true;
  }

  if (auto fade = request<C::Tuple<double, double, bool>>("Music", "fade"))
  {
    check (music, "No music to fade");
    double current_time = value<C::Double> (CLOCK__TIME);
    m_core.set_music_channels(music->tracks());
    for (std::size_t i = 0; i < music->tracks(); ++ i)
    {
      m_core.set_volume (music->core(i), i, volume * music->mix(i));
      m_core.fade(music->core(i), i, fade->get<1>() - current_time, fade->get<2>());
    }
    music->on() = true;
    remove("Music", "fade");
  }

  if ((receive("Music", "volume_changed") || volume_changed) && music)
    for (std::size_t i = 0; i < music->tracks(); ++ i)
      m_core.set_volume (music->core(i), i, volume * music->mix(i));

  if (music)
  {
    bool paused = (status()->is (PAUSED));

    if (paused && music->on())
    {
      if (status()->was (CUTSCENE))
        for (std::size_t i = 0; i < music->tracks(); ++ i)
          m_core.pause_music (music->core(i), i);
      else
        for (std::size_t i = 0; i < music->tracks(); ++ i)
          m_core.set_volume(music->core(i), i, 0.15 * volume * music->mix(i));
      music->on() = false;
    }
    else if (!paused && !music->on())
    {
      if (status()->is (CUTSCENE))
        for (std::size_t i = 0; i < music->tracks(); ++ i)
          m_core.resume_music(music->core(i), i);
      else
        for (std::size_t i = 0; i < music->tracks(); ++ i)
          m_core.set_volume (music->core(i), i, volume * music->mix(i));
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

  // Steps
  for (C::Handle ev : components("walk_start_time"))
  {
    auto stime = C::cast<C::Double>(ev);
    double start_time = stime->value();
    double current_time = value<C::Double>(CLOCK__TIME);

    double period = (4. / Config::animation_fps);

    if (current_time - start_time >= period || start_time == current_time)
    {
      Point point = value<C::Position>(ev->entity(), "position")
                  - value<C::Absolute_position>(CAMERA__POSITION);
      double panning = 1. - (point.x() / Config::world_width);
      m_core.play_sound (get<C::Sound> ("Step" , "sound")->core(),
                         value<C::Int>("Sounds", "volume") / 10., panning);
      stime->set(current_time);
    }
  }

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
