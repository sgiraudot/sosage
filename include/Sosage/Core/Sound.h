/*
  [include/Sosage/Core/Sound.h]
  Abstraction file for third party library handling sound.

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

#ifndef SOSAGE_CORE_SOUND_H
#define SOSAGE_CORE_SOUND_H

#include <Sosage/Config/platform.h>

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

#include <Sosage/Third_party/SDL_mixer.h>
namespace Sosage::Core
{
using Sound = Third_party::SDL_mixer;
} // namespace Sosage::Core

#else

namespace Sosage::Core
{

class No_sound
{
public:

  using Sound = int;
  using Music = int;

  No_sound() { }
  ~No_sound() { }

  static Music load_music (const std::string& file_name) { return 0; }
  static Sound load_sound (const std::string& file_name) { return 0; }

  static void delete_music (const Music& music) { }
  static void delete_sound (const Sound& sound) { }

  void start_music (const Music& music, double percentage) {}
  void stop_music() {}
  void fade (const Music& music, double time, bool in) {}
  void set_volume (double percentage) {}
  void pause_music (const Music& music) {}
  void resume_music (const Music& music) {}
  void play_sound (const Sound& sound, double percentage, double panning = 0.5) {}

};

using Sound = No_sound;

} // namespace Sosage::Core

#endif

#endif
