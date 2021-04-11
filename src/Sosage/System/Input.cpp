/*
  [src/Sosage/System/Graphic.cpp]
  Reads and interprets user input (keyboard, mouse, touchscreen).

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

#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Status.h>
#include <Sosage/System/Input.h>

namespace Sosage::System
{

namespace C = Component;

Input::Input (Content& content)
  : Base (content)
  , m_core()
  , m_keys_on(NUMBER_OF_EVENT_VALUES, false)
  , m_x(0)
  , m_y(0)
{
}

void Input::run()
{
  auto status = get<C::Status>(GAME__STATUS);

  bool keyboard_used = false;
  bool mouse_used = false;
  bool touchscreen_used = false;
  bool gamepad_used = false;

  while (Event ev = m_core.next_event ())
  {
    if ((ev.type() == KEY_DOWN || ev.type() == KEY_UP)
        && (ev.value() == UP_ARROW || ev.value() == RIGHT_ARROW ||
            ev.value() == LEFT_ARROW || ev.value() == DOWN_ARROW))
      keyboard_used = true;
    else if (ev.type() == MOUSE_DOWN || ev.type() == MOUSE_UP)
      mouse_used = true;
    else if (ev.type() == TOUCH_DOWN || ev.type() == TOUCH_MOVE || ev.type() == TOUCH_UP)
      touchscreen_used = true;
    else if (ev.type() == NEW_GAMEPAD || ev.type() == BUTTON_DOWN ||
             ev.type() == BUTTON_UP || ev.type() == STICK_MOVE)
      gamepad_used = true;

    m_current_events.emplace_back(ev);
  }

  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  Input_mode previous_mode = mode->value();
  if (touchscreen_used)
  {
    debug("Touchscreen mode activated");
    mode->set (TOUCHSCREEN);
  }
  else if (mouse_used)
  {
    debug("Mouse mode activated");
    mode->set (MOUSE);
  }
  else if (keyboard_used)
  {
    debug("Keyboard mode activated");
    mode->set (KEYBOARD);
  }
  else if (gamepad_used)
  {
    debug("Gamepad mode activated");
    mode->set (GAMEPAD);
  }

  if (previous_mode != mode->value())
    emit("Input_mode:changed");

  for (const Event& ev : m_current_events)
  {
    //debug("New event ", ev.to_string());

    if (ev == Event(KEY_UP, ESCAPE) ||
        ev == Event(KEY_UP, ANDROID_BACK))
      emit ("Game:escape");

    if (ev == Event(WINDOW, EXIT))
      emit ("Game:exit");

    if (ev == Event(KEY_UP, SPACE))
    {
      if (status->value() == PAUSED)
        status->pop();
      else
        status->push(PAUSED);
    }

    if (ev == Event(WINDOW, FOREGROUND)
        && status->value() == PAUSED)
        status->pop();
    if (ev == Event(WINDOW, BACKGROUND)
        && status->value() != PAUSED)
        status->push(PAUSED);

    if (status->value() == PAUSED)
      continue;

    if (ev == Event(KEY_UP, D))
      get<C::Boolean>("Game:debug")->toggle();

    if (ev == Event(KEY_UP, Sosage::C))
    {
      get<C::Boolean>("Game:console")->toggle();
    }

    if constexpr (!Config::emscripten) // Do not prevent web users to use F1/F2/etc
    {
      if (ev == Event(KEY_DOWN, ALT))
        key_on(ALT) = true;
      else if (ev == Event(KEY_UP, ALT))
        key_on(ALT) = false;
      else if (ev == Event(KEY_UP, ENTER) && key_on(ALT))
      {
        get<C::Boolean>("Window:fullscreen")->toggle();
        emit ("Window:toggle_fullscreen");
      }
    }
    if (ev == Event(WINDOW, RESIZED))
    {
      get<C::Int>("Window:width")->set(ev.x());
      get<C::Int>("Window:height")->set(ev.y());
      emit ("Window:rescaled");
    }

    // If paused, ignore mouse events
    if (status->value() == LOCKED)
      continue;

    if (mode->value() == MOUSE)
    {
      if (ev.type() == MOUSE_MOVE)
        get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));

      if (ev == Event(MOUSE_DOWN, LEFT))
      {
        get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
        emit ("Cursor:clicked");
        set<C::Boolean>("Click:left", true);
      }
      if (ev == Event(MOUSE_DOWN, RIGHT))
      {
        get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
        emit ("Cursor:clicked");
        set<C::Boolean>("Click:left", false);
      }
    }
    else if (mode->value() == TOUCHSCREEN)
    {
    }
    else if (mode->value() == KEYBOARD)
    {
      if (ev.type() == KEY_DOWN)
      {
        key_on(ev.value()) = true;
        if (ev.value() == TAB)
          set<C::Boolean>("Switch:right", true);
      }
      else if (ev.type() == KEY_UP)
        key_on(ev.value()) = false;
    }
    else // if (mode->value() == GAMEPAD)
    {
      if (ev == Event (STICK_MOVE, LEFT))
      {
        if (ev.y() == Config::no_value)
          m_x = (ev.x() + 0.5) / Config::stick_max;
        if (ev.x() == Config::no_value)
          m_y = (ev.y() + 0.5) / Config::stick_max;
      }
      else if (ev.type() == BUTTON_DOWN)
      {
        key_on(ev.value()) = true;
        if (ev.value() == LEFT_SHOULDER)
          set<C::Boolean>("Switch:right", false);
        else if (ev.value() == RIGHT_SHOULDER)
          set<C::Boolean>("Switch:right", true);
      }
      else if (ev.type() == BUTTON_UP)
        key_on(ev.value()) = false;
    }
  }

  if (mode->value() == KEYBOARD)
  {
    m_x = 0.;
    m_y = 0.;
    if (key_on(UP_ARROW)) m_y -= 1.;
    if (key_on(DOWN_ARROW)) m_y += 1.;
    if (key_on(LEFT_ARROW)) m_x -= 1.;
    if (key_on(RIGHT_ARROW)) m_x += 1.;
    set<C::Simple<Vector>>("Stick:direction", Vector(m_x, m_y));
  }
  else if (mode->value() == GAMEPAD)
  {
    /* TODO: add DPAD
    if (key_on(UP_ARROW)) m_y = -1.;
    if (key_on(DOWN_ARROW)) m_y = 1.;
    if (key_on(LEFT_ARROW)) m_x = 1.;
    if (key_on(RIGHT_ARROW)) m_x = -1.;
    */
    Vector vec (m_x, m_y);
    if (vec.length() > 0.25)
      vec.normalize();
    else
      vec = Vector(0,0);
    set<C::Simple<Vector>>("Stick:direction", vec);
  }

  m_current_events.clear();
}

} // namespace Sosage::System
