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

#include <SDL.h>
#include <Sosage/Utils/Event.h>
#include <Sosage/Utils/Gamepad_info.h>

#include <utility>
#include <unordered_map>

namespace Sosage
{

namespace Config
{
constexpr int no_value = std::numeric_limits<int>::max();
constexpr int deadzone = 6000;
} // namespace Config

namespace Third_party
{

class SDL_events
{
  std::unordered_map<SDL_EventType, Event_type> m_type_map;

public:

  SDL_events();

  ~SDL_events();

  Event next_event();

  Gamepad_info gamepad_info() const;

private:

  Event mouse_event (const Event_type& type, const SDL_Event& ev) const;
  Event keyboard_event (const Event_type& type, const SDL_Event& ev) const;
  Event touch_event (const Event_type& type, const SDL_Event& ev) const;
  Event gamepad_event (const Event_type& type, const SDL_Event& ev) const;
};

} // namespace Third_party

} // namespace Sosage

#endif // SOSAGE_THIRD_PARTY_SDL_EVENTS_H
