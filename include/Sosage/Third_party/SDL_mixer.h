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

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

#include <SDL_mixer.h>

#include <string>

namespace Sosage::Third_party
{

class SDL_mixer
{
public:

  typedef Mix_Music* Music;
  typedef Mix_Chunk* Sound;

private:

public:

  SDL_mixer ();
  ~SDL_mixer ();

  static Music load_music (const std::string& file_name);
  static Sound load_sound (const std::string& file_name);

  static void delete_music (const Music& music);
  static void delete_sound (const Sound& sound);

  void start_music (const Music& music);
  void stop_music();
  void fade (const Music& music, double time, bool in);
  void set_volume (double percentage);
  void pause_music (const Music& music);
  void resume_music (const Music& music);
  void play_sound (const Sound& sound);
};

} // namespace Sosage::Third_party

#endif

#endif // SOSAGE_THIRD_PARTY_SDL_H
