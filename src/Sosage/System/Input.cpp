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
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Status.h>
#include <Sosage/System/Input.h>

namespace Sosage::System
{

Input::Input (Content& content)
  : m_content (content)
  , m_core()
  , m_alt(false)
{
}

void Input::run()
{
  while (Event ev = m_core.next_event
         (m_content.get<Component::Int>("interface:width")->value(),
          m_content.get<Component::Int>("interface:height")->value()))
  {
    // Quit on: interface X-cross / Escape key / Q key
    if (ev == Event(WINDOW, QUIT) ||
        ev == Event(KEY_UP, EXIT) ||
        ev == Event(KEY_UP, Q) ||
        ev == Event(KEY_UP, ANDROID_BACK))
      m_content.set<Component::Event>("game:exit");

    auto status = m_content.get<Component::Status>(GAME__STATUS);
    if (status->value() == LOADING)
      return;

    if (ev == Event(KEY_UP, SPACE))
    {
      if (status->value() == PAUSED)
        status->pop();
      else
        status->push(PAUSED);
    }

    if (status->value() == PAUSED)
      continue;

    if (ev == Event(KEY_UP, D))
      m_content.get<Component::Boolean>("game:debug")->toggle();

    if (ev == Event(KEY_UP, C))
    {
      m_content.get<Component::Boolean>("game:console")->toggle();
    }

    if (ev == Event(KEY_UP, F1))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::AUTO);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (ev == Event(KEY_UP, F2))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::WIDESCREEN);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (ev == Event(KEY_UP, F3))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::STANDARD);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (ev == Event(KEY_UP, F4))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::SQUARE);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (ev == Event(KEY_UP, F5))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::PORTRAIT);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (ev == Event(KEY_DOWN, ALT))
      m_alt = true;
    else if (ev == Event(KEY_UP, ALT))
      m_alt = false;
    else if (ev == Event(KEY_UP, ENTER) && m_alt)
    {
      m_content.get<Component::Boolean>("window:fullscreen")->toggle();
      m_content.set<Component::Event>("window:toggle_fullscreen");
    }

    if (ev == Event(WINDOW, RESIZED))
    {
      m_content.get<Component::Int>("window:width")->set(ev.x());
      m_content.get<Component::Int>("window:height")->set(ev.y());
      m_content.set<Component::Event>("window:rescaled");
    }

    // If paused, ignore mouse events
    if (status->value() == LOCKED)
      continue;

    if (m_content.get<Component::Boolean>("interface:virtual_cursor")->value())
    {
      if (ev.type() == CURSOR_DOWN)
      {
        m_virtual_cursor.down = true;
        m_virtual_cursor.has_moved = false;
        m_virtual_cursor.time = Time::now();
        m_virtual_cursor.x = ev.x();
        m_virtual_cursor.y = ev.y();

        const Point& pos = m_content.get<Component::Position>(CURSOR__POSITION)->value();
        m_virtual_cursor.cursor_x = pos.x();
        m_virtual_cursor.cursor_y = pos.y();
      }

      if (ev.type() == CURSOR_UP)
      {
        m_virtual_cursor.down = false;

        if (!m_virtual_cursor.has_moved)
        {
          m_content.set<Component::Event>("cursor:clicked");
          m_content.set<Component::Boolean>("click:left", true);
        }
      }

      if (m_virtual_cursor.down &&
          ev.type() == CURSOR_MOVE)
      {
        auto pos = m_content.get<Component::Position>(CURSOR__POSITION);
        pos->set(Point(m_virtual_cursor.cursor_x + ev.x() - m_virtual_cursor.x,
                       m_virtual_cursor.cursor_y + ev.y() - m_virtual_cursor.y));
        if (!m_virtual_cursor.has_moved &&
            distance(pos->value().x(), pos->value().y(),
                     m_virtual_cursor.cursor_x,
                     m_virtual_cursor.cursor_y) < Config::virtual_cursor_sensitivity)
          m_virtual_cursor.has_moved = true;
      }

    }
    else // regular cursor
    {
      if (ev.type() == CURSOR_MOVE)
        m_content.get<Component::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));

      if (ev == Event(CURSOR_DOWN, LEFT))
      {
        m_content.get<Component::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
        m_content.set<Component::Event>("cursor:clicked");
        m_content.set<Component::Boolean>("click:left", true);
      }
      if (ev == Event(CURSOR_DOWN, RIGHT))
      {
        m_content.get<Component::Position>
            (CURSOR__POSITION)->set(Point(ev.x(), ev.y()));
        m_content.set<Component::Event>("cursor:clicked");
        m_content.set<Component::Boolean>("click:left", false);
      }
    }
  }
}

} // namespace Sosage::System
