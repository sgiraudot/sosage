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
#include <Sosage/Utils/conversions.h>

#include <iostream>

namespace Sosage::Third_party
{

SDL_events::SDL_events ()
  : m_type_map
    ({ {SDL_MOUSEBUTTONDOWN, MOUSE_DOWN},
       {SDL_MOUSEMOTION, MOUSE_MOVE},
       {SDL_MOUSEBUTTONUP, MOUSE_UP},
       {SDL_FINGERDOWN, TOUCH_DOWN},
       {SDL_FINGERMOTION, TOUCH_MOVE},
       {SDL_FINGERUP, TOUCH_UP},
       {SDL_CONTROLLERBUTTONDOWN, BUTTON_DOWN},
       {SDL_CONTROLLERBUTTONUP, BUTTON_UP},
       {SDL_CONTROLLERAXISMOTION, STICK_MOVE},
       {SDL_KEYDOWN, KEY_DOWN},
       {SDL_KEYUP, KEY_UP} })
{ }

SDL_events::~SDL_events ()
{

}

Event SDL_events::next_event ()
{
  SDL_Event ev;
  if (SDL_PollEvent(&ev) != 1)
    return Event();

  if (ev.type == SDL_QUIT || ev.type == SDL_APP_TERMINATING)
    return Event (WINDOW, EXIT);
  if (ev.type == SDL_APP_WILLENTERBACKGROUND)
    return Event (WINDOW, BACKGROUND);
  if (ev.type == SDL_APP_DIDENTERFOREGROUND)
    return Event (WINDOW, FOREGROUND);
  if (ev.type == SDL_WINDOWEVENT &&
      ev.window.event == SDL_WINDOWEVENT_RESIZED)
    return Event (WINDOW, RESIZED, ev.window.data1, ev.window.data2);
  if (ev.type == SDL_CONTROLLERDEVICEADDED)
  {
    SDL_GameControllerOpen(ev.cdevice.which);
    return Event (NEW_GAMEPAD, NONE, ev.cdevice.which);
  }

  auto iter = m_type_map.find(SDL_EventType(ev.type));
  if (iter == m_type_map.end())
    return Event();

  Event_type type = iter->second;
  if (type == KEY_DOWN || type == KEY_UP)
    return keyboard_event(type, ev);
  if (type == TOUCH_DOWN || type == TOUCH_MOVE || type == TOUCH_UP)
    return touch_event(type, ev);
  if (type == MOUSE_DOWN || type == MOUSE_MOVE || type == MOUSE_UP)
    return mouse_event(type, ev);
  if (type == BUTTON_DOWN || type == BUTTON_UP || type == STICK_MOVE)
    return gamepad_event(type, ev);

  return Event();
}

Gamepad_type SDL_events::gamepad_type() const
{
  for (int i = 0; i < SDL_NumJoysticks(); ++ i)
    if (SDL_IsGameController(i))
    {
      SDL_GameController* controller = SDL_GameControllerOpen(i);
      if (controller)
      {
        const char* name_str = SDL_GameControllerName(controller);
        if (name_str)
        {
          std::string name = name_str;
//          std::cerr << "CONTROLLER NAME = " << name << std::endl;
          if (contains (name, "Nintendo"))
            return JAPAN;
          if (contains (name, "Steam") || contains (name, "X-Box"))
            return USA;
        }
        break;
      }
  }

  // Default controller, no label
  return NO_LABEL;
}

Event SDL_events::mouse_event (const Event_type& type, const SDL_Event& ev) const
{
  Event_value value;
  if (ev.button.button == SDL_BUTTON_LEFT)
    value = LEFT;
  else if (ev.button.button == SDL_BUTTON_RIGHT)
    value = RIGHT;
  if (type == MOUSE_MOVE)
    return Event (type, value, ev.motion.x, ev.motion.y);
  // else
  return Event (type, value, ev.button.x, ev.button.y);
}

Event SDL_events::keyboard_event (const Event_type& type, const SDL_Event& ev) const
{
  if (ev.key.keysym.sym == SDLK_LALT)
    return Event (type, ALT);
  if (ev.key.keysym.sym == SDLK_LCTRL || ev.key.keysym.sym == SDLK_RCTRL)
    return Event (type, CTRL);
  if (ev.key.keysym.sym == SDLK_LSHIFT || ev.key.keysym.sym == SDLK_RSHIFT)
    return Event (type, SHIFT);
  if (ev.key.keysym.sym == SDLK_TAB)
    return Event (type, TAB);
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
Event SDL_events::touch_event (const Event_type& type, const SDL_Event& ev) const
{
  return Event (type, LEFT,
                int(ev.tfinger.x * Config::world_width),
                int(ev.tfinger.y * Config::world_height));
}
Event SDL_events::gamepad_event (const Event_type& type, const SDL_Event& ev) const
{
  if (type == STICK_MOVE)
  {
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
      return Event (type, LEFT, ev.caxis.value, Config::no_value);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
      return Event (type, LEFT, Config::no_value, ev.caxis.value);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
      return Event (type, RIGHT, ev.caxis.value, Config::no_value);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
      return Event (type, RIGHT, Config::no_value, ev.caxis.value);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERLEFT && ev.caxis.value > 0)
      return Event (BUTTON_DOWN, LEFT_SHOULDER);
    if (ev.caxis.axis == SDL_CONTROLLER_AXIS_TRIGGERRIGHT && ev.caxis.value > 0)
      return Event (BUTTON_DOWN, RIGHT_SHOULDER);
  }
  else
  {
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_UP)
      return Event (type, UP_ARROW);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_RIGHT)
      return Event (type, RIGHT_ARROW);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_LEFT)
      return Event (type, LEFT_ARROW);
    if (ev.cbutton.button == SDL_CONTROLLER_BUTTON_DPAD_DOWN)
      return Event (type, DOWN_ARROW);
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
  }
  return Event();
}

} // namespace Sosage::Third_party
