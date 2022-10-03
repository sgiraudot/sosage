/*
  [src/Sosage/Third_party/SDL_mixer.cpp]
  Wrapper for SDL library (sound system).

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

#include <Sosage/Config/config.h>
#include <Sosage/Third_party/SDL_mixer.h>
#include <Sosage/Utils/Asset_manager.h>

#include <Sosage/Utils/error.h>

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

namespace Sosage::Config
{
int sound_channels = MIX_CHANNELS;
}

namespace Sosage::Third_party
{

SDL_mixer::SDL_mixer()
  : m_music_channels(0), m_current_channel(0)
{
  int init = Mix_OpenAudio (44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
  check (init != -1, "Cannot initialized SDL Mixer (" + std::string(Mix_GetError() )+ ")");
  Mix_AllocateChannels (Config::sound_channels);
}

SDL_mixer::~SDL_mixer()
{
  Mix_CloseAudio ();
}

SDL_mixer::Music SDL_mixer::load_music (const std::string& file_name)
{
  return load_sound(file_name);
}

SDL_mixer::Sound SDL_mixer::load_sound (const std::string& file_name)
{
  Asset asset = Asset_manager::open(file_name);
  Mix_Chunk* sound = Mix_LoadWAV_RW (asset.base(), 0);
  asset.close();
  check (sound != nullptr, "Cannot load sound " + file_name);
  return sound;
}

void SDL_mixer::delete_music (SDL_mixer::Music& music)
{
  delete_sound(music);
}

void SDL_mixer::delete_sound (SDL_mixer::Sound& sound)
{
  Mix_FreeChunk (sound);
}

void SDL_mixer::set_music_channels (std::size_t nb)
{
  m_music_channels = nb;
  if (m_current_channel < int(nb))
    m_current_channel = int(nb);
}

void SDL_mixer::start_music (const SDL_mixer::Music& music, int channel, double volume)
{
  debug << "Start music with volume " << volume << "% (" << int(volume * Config::max_music_volume) << ")" << std::endl;
  Mix_Volume(channel, int(volume * Config::max_music_volume));
  Mix_PlayChannel (channel, music, -1);
}

void SDL_mixer::stop_music(int channel)
{
  debug << "Stop music" << std::endl;
  Mix_HaltChannel(channel);
}

void SDL_mixer::fade (const SDL_mixer::Music& music, int channel, double time, bool in)
{
  if (in)
  {
    debug << "Fade in music " << time << std::endl;
    Mix_FadeInChannel(channel, music, -1, int(1000 * time));
  }
  else
  {
    debug << "Fade out music" << std::endl;
    Mix_FadeOutChannel(channel, int(1000 * time));
  }
}

void SDL_mixer::set_volume (int channel, double percentage)
{
  debug << "Set volume to " << percentage << "% (" << int(percentage * Config::max_music_volume) << ")" << std::endl;
  Mix_Volume(channel, int(percentage * Config::max_music_volume));
}

void SDL_mixer::pause_music (int channel)
{
  debug << "Pause music" << std::endl;
  Mix_Pause (channel);
}

void SDL_mixer::resume_music (int channel)
{
  debug << "Resume music" << std::endl;
  Mix_Resume(channel);
}

void SDL_mixer::play_sound (const SDL_mixer::Sound& sound, double volume, double panning)
{
  int left = int(panning * 2 * volume * Config::max_music_volume);
  int right = int((1. - panning) * 2 * volume * Config::max_music_volume);
  Mix_VolumeChunk (sound, 255);
  Mix_SetPanning(m_current_channel, left, right);
  Mix_PlayChannel(m_current_channel, sound, 0);
  m_current_channel = m_music_channels + (m_current_channel + 1)
                      % (Config::sound_channels - m_music_channels);
}


} // namespace Sosage::Third_party

#endif
