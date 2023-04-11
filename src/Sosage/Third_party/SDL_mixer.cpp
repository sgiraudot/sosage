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

namespace Sosage::Third_party
{

std::array<bool, Config::sound_channels> SDL_mixer::m_available_channels;

SDL_mixer::SDL_mixer()
{
  debug << "Using vanilla SDL Mixer" << std::endl;
  int init = Mix_OpenAudio (44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 2048);
  check (init != -1, "Cannot initialized SDL Mixer (" + std::string(Mix_GetError() )+ ")");
  Mix_AllocateChannels (Config::sound_channels);

  for (std::size_t i = 0; i < Config::sound_channels; ++ i)
  {
    m_available_channels[i] = true;
    Mix_ChannelFinished([](int channel)
    {
      debug << "Release channel " << channel << std::endl;
      m_available_channels[channel] = true;
    });
  }

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
  m_music_channels.resize(nb);
  for (std::size_t i = 0; i < nb; ++ i)
  {
    int channel = reserve_channel();
    check (channel != -1, "No more sound channel available.");
    Mix_SetPanning(channel, 255, 255);
    m_music_channels[i] = channel;
  }
}

void SDL_mixer::start_music (const SDL_mixer::Music& music, int channel, double volume)
{
  debug << "Start music with volume " << volume << "% (" << int(volume * Config::max_music_volume) << ")" << std::endl;
  Mix_Volume(m_music_channels[channel], int(volume * Config::max_music_volume));
  Mix_PlayChannel (m_music_channels[channel], music, -1);
}

void SDL_mixer::stop_music(const SDL_mixer::Music&, int channel)
{
  debug << "Stop music" << std::endl;
  Mix_HaltChannel(m_music_channels[channel]);
}

void SDL_mixer::fade (const SDL_mixer::Music& music, int channel, double time, bool in)
{
  if (in)
  {
    debug << "Fade in music " << time << std::endl;
    Mix_FadeInChannel(m_music_channels[channel], music, -1, int(1000 * time));
  }
  else
  {
    debug << "Fade out music" << std::endl;
    Mix_FadeOutChannel(m_music_channels[channel], int(1000 * time));
  }
}

void SDL_mixer::set_volume (const SDL_mixer::Music&, int channel, double percentage)
{
  debug << "Set volume to " << percentage << "% (" << int(percentage * Config::max_music_volume) << ")" << std::endl;
  Mix_Volume(m_music_channels[channel], int(percentage * Config::max_music_volume));
}

void SDL_mixer::pause_music (const SDL_mixer::Music&, int channel)
{
  debug << "Pause music" << std::endl;
  Mix_Pause (m_music_channels[channel]);
}

void SDL_mixer::resume_music (const SDL_mixer::Music&, int channel)
{
  debug << "Resume music" << std::endl;
  Mix_Resume(m_music_channels[channel]);
}

void SDL_mixer::play_sound (const SDL_mixer::Sound& sound, double volume, double panning)
{
  int channel = reserve_channel();
  int left = int((panning) * Config::max_panning);
  int right= int((1. - panning) * Config::max_panning);
  Mix_Volume (channel, volume * Config::max_music_volume);
  Mix_SetPanning(channel, left, right);
  Mix_PlayChannel(channel, sound, 0);
}

int SDL_mixer::reserve_channel()
{
  for (std::size_t i = 0; i < Config::sound_channels; ++ i)
    if (m_available_channels[i])
    {
      debug << "Reserve channel " << i << std::endl;
      m_available_channels[i] = false;
      return i;
    }
  return -1;
}

} // namespace Sosage::Third_party

#endif
