/*
  [include/Sosage/Third_party/SDL_mixer.h]
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

#ifndef SOSAGE_THIRD_PARTY_SDL_MIXER_H
#define SOSAGE_THIRD_PARTY_SDL_MIXER_H

#include <Sosage/Config/platform.h>

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

#include <SDL_mixer.h>

#include <array>
#include <string>
#include <vector>

namespace Sosage
{

namespace Config
{
constexpr int max_music_volume = 128;
constexpr int max_panning = 255;
constexpr int sound_channels = 16;
} // namespace Config

namespace Third_party
{

class SDL_mixer
{
public:

  using Music = Mix_Chunk*;
  using Sound = Mix_Chunk*;

private:

  static std::array<bool, Config::sound_channels> m_available_channels;
  std::vector<int> m_music_channels;

public:

  SDL_mixer ();
  ~SDL_mixer ();

  static Music load_music (const std::string& file_name);
  static Sound load_sound (const std::string& file_name);

  static void delete_music (Music& music);
  static void delete_sound (Sound& sound);

  void set_music_channels (std::size_t nb);
  void start_music (const Music& music, int channel, double volume);
  void stop_music(const Music& music, int channel);
  void fade (const Music& music, int channel, double time, bool in, double position = 0.);
  void set_volume (const Music& music, int channel, double percentage);
  void pause_music (const Music& music, int channel);
  void resume_music (const Music& music, int channel);
  void play_sound (const Sound& sound, double volume, double panning = 0.5);

  double position (const Music& music) const;

private:

  int reserve_channel();
};

} // namespace Third_party

} // namespace Sosage

#endif

#endif // SOSAGE_THIRD_PARTY_SDL_MIXER_H
