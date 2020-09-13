/*
  [src/Sosage/Third_party/SDL_events.cpp]
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

#include <Sosage/Config/config.h>
#include <Sosage/Config/platform.h>
#include <Sosage/Third_party/SDL_events.h>

#include <iostream>

namespace Sosage::Third_party
{

SDL_events::SDL_events ()
{

}

SDL_events::~SDL_events ()
{

}

Event SDL_events::next_event (int interface_width, int interface_height)
{
  SDL_Event ev;
  if (SDL_PollEvent(&ev) != 1)
    return Event();

  // Exit button
  if (ev.type == SDL_QUIT)
    return Event (WINDOW, QUIT);

  // Window resized
  if (ev.type == SDL_WINDOWEVENT &&
      ev.window.event == SDL_WINDOWEVENT_RESIZED)
    return Event (WINDOW, RESIZED, ev.window.data1, ev.window.data2);

  Event::Type type = EMPTY;
  Event::Value value = NONE;

  // Mouse/finger

  bool is_mouse = false;
  bool is_finger = false;
  if (ev.type == SDL_MOUSEBUTTONDOWN)
  {
    type = CURSOR_DOWN;
    is_mouse = true;
  }
  else if (ev.type == SDL_MOUSEMOTION)
  {
    type = CURSOR_MOVE;
    is_mouse = true;
  }
  else if (ev.type == SDL_MOUSEBUTTONUP)
  {
    type = CURSOR_UP;
    is_mouse = true;
  }
  else if (ev.type == SDL_FINGERDOWN)
  {
    type = CURSOR_DOWN;
    is_finger = true;
  }
  else if (ev.type == SDL_FINGERMOTION)
  {
    type = CURSOR_MOVE;
    is_finger = true;
  }
  else if (ev.type == SDL_FINGERUP)
  {
    type = CURSOR_UP;
    is_finger = true;
  }

  if (is_mouse)
  {
    // For some reason, SDL generates fake mouse events that duplicate finger events and that
    // mess up everything, so let's just ignore mouse events on Android…
    if constexpr (Config::android)
      return Event();

    if (ev.button.button == SDL_BUTTON_LEFT)
      value = LEFT;
    else if (ev.button.button == SDL_BUTTON_RIGHT)
      value = RIGHT;
    if (ev.type == SDL_MOUSEMOTION)
      return Event (type, value, ev.motion.x, ev.motion.y);
    // else
    return Event (type, value, ev.button.x, ev.button.y);
  }

  if (is_finger)
  {
    return Event (type, LEFT,
                  ev.tfinger.x * (Config::world_width + interface_width),
                  ev.tfinger.y * (Config::world_height + interface_height));
  }

  // Keys
  if (ev.type == SDL_KEYDOWN)
    type = KEY_DOWN;
  else if (ev.type == SDL_KEYUP)
    type = KEY_UP;
  else
    return Event();


  if (ev.key.keysym.sym == SDLK_LALT)
    return Event (type, ALT);
  if (ev.key.keysym.sym == SDLK_AC_BACK)
    return Event (type, ANDROID_BACK);
  if (ev.key.keysym.sym == SDLK_RETURN)
      return Event (type, ENTER);
  if (ev.key.keysym.sym == SDLK_ESCAPE)
    return Event (type, EXIT);
  if (ev.key.keysym.sym == SDLK_SPACE)
    return Event (type, SPACE);
  if (SDLK_a <= ev.key.keysym.sym && ev.key.keysym.sym <= SDLK_z)
    return Event (type, Event::Value(A + (ev.key.keysym.sym - SDLK_a)));
  if (SDLK_F1 <= ev.key.keysym.sym && ev.key.keysym.sym <= SDLK_F12)
    return Event (type, Event::Value(F1 + (ev.key.keysym.sym - SDLK_F1)));

  return Event();
}

} // namespace Sosage::Third_party

