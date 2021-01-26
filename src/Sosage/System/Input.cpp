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
  , m_alt(false)
{
}

void Input::run()
{
  auto status = get<C::Status>(GAME__STATUS);

  while (Event ev = m_core.next_event ())
  {
    debug("Loop ", i, " -> new event ", ev.to_string());

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
        m_alt = true;
      else if (ev == Event(KEY_UP, ALT))
        m_alt = false;
      else if (ev == Event(KEY_UP, ENTER) && m_alt)
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

    if (get<C::Boolean>("Interface:virtual_cursor")->value())
    {
      if (ev.type() == CURSOR_DOWN)
      {
        m_virtual_cursor.down = true;
        m_virtual_cursor.has_moved = false;
        m_virtual_cursor.time = Time::now();
        m_virtual_cursor.x = ev.x();
        m_virtual_cursor.y = ev.y();

        const Point& pos = get<C::Position>(CURSOR__POSITION)->value();
        m_virtual_cursor.cursor_x = pos.x();
        m_virtual_cursor.cursor_y = pos.y();
      }

      if (ev.type() == CURSOR_UP)
      {
        if (m_virtual_cursor.down) // should always be true except if we just activated virtual cursor
        {
          m_virtual_cursor.down = false;

          if (!m_virtual_cursor.has_moved)
          {
            emit ("Cursor:clicked");
            set<C::Boolean>("Click:left", true);
          }
        }
      }

      if (m_virtual_cursor.down &&
          ev.type() == CURSOR_MOVE)
      {
        auto pos = get<C::Position>(CURSOR__POSITION);
        pos->set(Point(m_virtual_cursor.cursor_x + Config::virtual_cursor_speed * (ev.x() - m_virtual_cursor.x),
                       m_virtual_cursor.cursor_y + Config::virtual_cursor_speed * (ev.y() - m_virtual_cursor.y)));
        if (!m_virtual_cursor.has_moved &&
            distance(pos->value().x(), pos->value().y(),
                     m_virtual_cursor.cursor_x,
                     m_virtual_cursor.cursor_y) > Config::virtual_cursor_sensitivity)
          m_virtual_cursor.has_moved = true;
      }

    }
    else // regular cursor
    {
      if (ev.type() == CURSOR_MOVE)
        get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));

      if (ev == Event(CURSOR_DOWN, LEFT))
      {
        get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
        emit ("Cursor:clicked");
        set<C::Boolean>("Click:left", true);
      }
      if (ev == Event(CURSOR_DOWN, RIGHT))
      {
        get<C::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
        emit ("Cursor:clicked");
        set<C::Boolean>("Click:left", false);
      }
    }
  }
}

} // namespace Sosage::System
