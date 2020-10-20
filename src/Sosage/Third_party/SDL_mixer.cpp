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
#include <Sosage/Utils/error.h>

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

namespace Sosage::Third_party
{

SDL_mixer::SDL_mixer()
{
  int init = Mix_OpenAudio (44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
  check (init != -1, "Cannot initialized SDL Mixer");
  Mix_AllocateChannels (10);
}

SDL_mixer::~SDL_mixer()
{
  Mix_CloseAudio ();
}

SDL_mixer::Music SDL_mixer::load_music (const std::string& file_name)
{
  Mix_Music* music = Mix_LoadMUS (file_name.c_str());
  Mix_VolumeMusic(64);
  check (music != nullptr, "Cannot load music " + file_name);
  return music;
}

SDL_mixer::Sound SDL_mixer::load_sound (const std::string& file_name)
{
  Mix_Chunk* sound = Mix_LoadWAV (file_name.c_str());
  check (sound != nullptr, "Cannot load sound " + file_name);
  return sound;
}

void SDL_mixer::delete_music (const SDL_mixer::Music& music)
{
  Mix_FreeMusic (music);
}

void SDL_mixer::delete_sound (const SDL_mixer::Sound& sound)
{
  Mix_FreeChunk (sound);
}

void SDL_mixer::start_music (const SDL_mixer::Music& music)
{
  Mix_VolumeMusic(Config::max_music_volume / 2);
  Mix_PlayMusic (music, -1);
}

void SDL_mixer::set_volume (double percentage)
{
  Mix_VolumeMusic(int(percentage * Config::max_music_volume));
}

void SDL_mixer::pause_music (const SDL_mixer::Music&)
{
  Mix_VolumeMusic(Config::max_music_volume / 6);
//  Mix_PauseMusic();
}

void SDL_mixer::resume_music (const SDL_mixer::Music&)
{
  Mix_VolumeMusic(Config::max_music_volume / 2);
//  Mix_ResumeMusic();
}

void SDL_mixer::play_sound (const SDL_mixer::Sound& sound)
{
  Mix_PlayChannel(-1, sound, 0);
}


} // namespace Sosage::Third_party

#endif
