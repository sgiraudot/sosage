/*
  [src/Sosage/Third_party/SDL_mixer_ext.cpp]
  Wrapper for SDL library (extended sound system).

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
#include <Sosage/Third_party/SDL_mixer_ext.h>
#include <Sosage/Utils/Asset_manager.h>

#include <Sosage/Utils/error.h>

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER_EXT

namespace Sosage::Third_party
{

std::array<bool, Config::sound_channels> SDL_mixer_ext::m_available_channels;

SDL_mixer_ext::SDL_mixer_ext()
{
  debug << "Using SDL Mixer Ext" << std::endl;
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

SDL_mixer_ext::~SDL_mixer_ext()
{
  Mix_CloseAudio ();
}

SDL_mixer_ext::Music SDL_mixer_ext::load_music (const std::string& file_name)
{
  Asset asset = Asset_manager::open(file_name);
  Mix_Music* music = Mix_LoadMUS_RW (asset.base(), 0);
  check (music != nullptr, "Cannot load music " + file_name);
  return std::make_pair (music, asset);
}

SDL_mixer_ext::Sound SDL_mixer_ext::load_sound (const std::string& file_name)
{
  Asset asset = Asset_manager::open(file_name);
  Mix_Chunk* sound = Mix_LoadWAV_RW (asset.base(), 0);
  asset.close();
  check (sound != nullptr, "Cannot load sound " + file_name);
  return sound;
}

void SDL_mixer_ext::delete_music (SDL_mixer_ext::Music& music)
{
  Mix_FreeMusic(music.first);
  music.second.close();
}

void SDL_mixer_ext::delete_sound (SDL_mixer_ext::Sound& sound)
{
  Mix_FreeChunk (sound);
}

void SDL_mixer_ext::set_music_channels (std::size_t)
{
}

void SDL_mixer_ext::start_music (const SDL_mixer_ext::Music& music, int, double volume)
{
  debug << "Start music with volume " << volume << "% (" << int(volume * Config::max_music_volume) << ")" << std::endl;
  Mix_VolumeMusicStream (music.first, int(volume * Config::max_music_volume));
  Mix_PlayMusicStream (music.first, -1);
}

void SDL_mixer_ext::stop_music(const SDL_mixer_ext::Music& music, int)
{
  debug << "Stop music" << std::endl;
  Mix_HaltMusicStream (music.first);
}

void SDL_mixer_ext::fade (const SDL_mixer_ext::Music& music, int,
                          double time, bool in, double position)
{
  if (in)
  {
    debug << "Fade in music " << time << std::endl;
    if (position == 0)
      Mix_FadeInMusicStream(music.first, -1, int(1000 * time));
    else
      Mix_FadeInMusicStreamPos (music.first, -1, int(1000 * time), position);
  }
  else
  {
    debug << "Fade out music" << std::endl;
    Mix_FadeOutMusicStream(music.first, int(1000 * time));
  }
}

void SDL_mixer_ext::set_volume (const SDL_mixer_ext::Music& music, int, double percentage)
{
  debug << "Set volume to " << percentage << "% (" << int(percentage * Config::max_music_volume) << ")" << std::endl;
  Mix_VolumeMusicStream (music.first, int(percentage * Config::max_music_volume));
}

void SDL_mixer_ext::pause_music (const SDL_mixer_ext::Music& music, int)
{
  debug << "Pause music" << std::endl;
  Mix_PauseMusicStream (music.first);
}

void SDL_mixer_ext::resume_music (const SDL_mixer_ext::Music& music, int)
{
  debug << "Resume music" << std::endl;
  Mix_ResumeMusicStream(music.first);
}

void SDL_mixer_ext::play_sound (const SDL_mixer_ext::Sound& sound, double volume, double panning)
{
  int channel = reserve_channel();
  int left = int((panning) * Config::max_panning);
  int right= int((1. - panning) * Config::max_panning);
  Mix_Volume (channel, volume * Config::max_music_volume);
  Mix_SetPanning(channel, left, right);
  Mix_PlayChannel(channel, sound, 0);
}

double SDL_mixer_ext::position (const SDL_mixer_ext::Music& music) const
{
  return Mix_GetMusicPosition (music.first);
}

int SDL_mixer_ext::reserve_channel()
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
