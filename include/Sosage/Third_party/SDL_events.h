/*
  [include/Sosage/Third_party/SDL_events.h]
  Wrapper for SDL library (event handling).

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

#ifndef SOSAGE_THIRD_PARTY_SDL_EVENTS_H
#define SOSAGE_THIRD_PARTY_SDL_EVENTS_H

#include <utility>

#include <SDL.h>

namespace Sosage::Third_party
{

class SDL_events
{
public:

  typedef SDL_Event Event;
  
private:
  
public:

  SDL_events();

  ~SDL_events();

  bool next_event(Event& ev);

  bool is_exit (const Event& ev);
  bool is_pause (const Event& ev);
  bool is_debug (const Event& ev);
  bool is_console (const Event& ev);
  bool is_f1 (const Event& ev);
  bool is_f2 (const Event& ev);
  bool is_f3 (const Event& ev);
  bool is_f4 (const Event& ev);
  bool is_f5 (const Event& ev);
  bool is_alt_on (const Event& ev);
  bool is_alt_off (const Event& ev);
  bool is_enter (const Event& ev);
  bool is_left_click (const Event& ev);
  bool is_right_click(const Event& ev);
  bool is_mouse_motion (const Event& ev);
  bool is_window_resized (const Event& ev);
  std::pair<int, int> mouse_position (const Event& ev,
                                      int interface_width,
                                      int interface_height);
  std::pair<int, int> window_size (const Event& ev);
};

} // namespace Sosage::Third_party

#endif // SOSAGE_THIRD_PARTY_SDL_EVENTS_H
