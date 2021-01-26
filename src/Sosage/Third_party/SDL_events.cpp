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

Event SDL_events::next_event ()
{
  SDL_Event ev;
  if (SDL_PollEvent(&ev) != 1)
    return Event();

  // Exit button
  if (ev.type == SDL_QUIT || ev.type == SDL_APP_TERMINATING)
    return Event (WINDOW, EXIT);

  if (ev.type == SDL_APP_WILLENTERBACKGROUND)
    return Event (WINDOW, BACKGROUND);
  if (ev.type == SDL_APP_DIDENTERFOREGROUND)
    return Event (WINDOW, FOREGROUND);

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
                  int(ev.tfinger.x * Config::world_width),
                  int(ev.tfinger.y * Config::world_height));
  }

  if (ev.type == SDL_CONTROLLERBUTTONDOWN)
  {
    type = BUTTON_DOWN;

    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_A)
      return Event (type, SOUTH);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_B)
      return Event (type, EAST);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_X)
      return Event (type, WEST);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_Y)
      return Event (type, NORTH);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
      return Event (type, LEFT_SHOULDER);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
      return Event (type, RIGHT_SHOULDER);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_START)
      return Event (type, START);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_BACK)
      return Event (type, SELECT);
    return Event (type, NONE);
  }
  if (ev.type == SDL_CONTROLLERAXISMOTION)
  {
    type = STICK_MOVE;
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
      return Event (type, LEFT, ev.caxis.value, 0);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
      return Event (type, LEFT, 0, ev.caxis.value);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
      return Event (type, RIGHT, ev.caxis.value, 0);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
      return Event (type, RIGHT, 0, ev.caxis.value);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT)
      return Event (BUTTON_DOWN, LEFT_SHOULDER);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT)
      return Event (BUTTON_DOWN, RIGHT_SHOULDER);
    return Event(type, NONE);
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
    return Event (type, ESCAPE);
  if (ev.key.keysym.sym == SDLK_SPACE)
    return Event (type, SPACE);
  if (ev.key.keysym.sym == SDLK_UP)
    return Event (type, UP_ARROW);
  if (ev.key.keysym.sym == SDLK_RIGHT)
    return Event (type, RIGHT_ARROW);
  if (ev.key.keysym.sym == SDLK_LEFT)
    return Event (type, LEFT_ARROW);
  if (ev.key.keysym.sym == SDLK_DOWN)
    return Event (type, DOWN_ARROW);
  if (SDLK_a <= ev.key.keysym.sym && ev.key.keysym.sym <= SDLK_z)
    return Event (type, Event::Value(A + (ev.key.keysym.sym - SDLK_a)));
  if (SDLK_F1 <= ev.key.keysym.sym && ev.key.keysym.sym <= SDLK_F12)
    return Event (type, Event::Value(F1 + (ev.key.keysym.sym - SDLK_F1)));

  return Event();
}

} // namespace Sosage::Third_party

