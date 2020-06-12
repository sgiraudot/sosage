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
  Core::Input::Event ev;

  while (m_core.next_event(ev))
  {
    if (m_core.is_exit(ev))
      m_content.set<Component::Event>("game:exit");

    if (m_core.is_pause(ev))
      m_content.get<Component::Boolean>("game:paused")->toggle();

    auto paused = m_content.request<Component::Boolean>("game:paused");

    if (paused && paused->value())
      continue;

    if (m_core.is_debug(ev))
      m_content.get<Component::Boolean>("game:debug")->toggle();
    
    if (m_core.is_console(ev))
      m_content.get<Component::Boolean>("game:console")->toggle();

    if (m_core.is_f1(ev))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::AUTO);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (m_core.is_f2(ev))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::WIDESCREEN);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (m_core.is_f3(ev))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::STANDARD);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (m_core.is_f4(ev))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::SQUARE);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (m_core.is_f5(ev))
    {
      m_content.get<Component::Int>("interface:layout")->set(Config::PORTRAIT);
      m_content.set<Component::Event>("window:rescaled");
    }
    else if (m_core.is_alt_on(ev))
      m_alt = true;
    else if (m_core.is_alt_off(ev))
      m_alt = false;
    else if (m_core.is_enter(ev))
    {
      m_content.get<Component::Boolean>("window:fullscreen")->toggle();
      m_content.set<Component::Event>("window:toggle_fullscreen");
    }

    
    if (m_core.is_window_resized(ev))
    {
      std::pair<int, int> window_size
        = m_core.window_size(ev);
      m_content.get<Component::Int>("window:width")->set(window_size.first);
      m_content.get<Component::Int>("window:height")->set(window_size.second);
      m_content.set<Component::Event>("window:rescaled");
    }

    // If paused, ignore mouse events
    if (m_content.get<Component::String>("game:status")->value() == "locked")
      continue;
  
    if (m_core.is_mouse_motion(ev))
      m_content.set<Component::Position>
        ("cursor:position",
         Point(m_core.mouse_position
               (ev,
                m_content.get<Component::Int>("interface:width")->value(),
                m_content.get<Component::Int>("interface:height")->value())));
    
    if (m_core.is_left_click(ev))
    {
      m_content.set<Component::Position>
        ("cursor:position",
         Point(m_core.mouse_position
               (ev,
                m_content.get<Component::Int>("interface:width")->value(),
                m_content.get<Component::Int>("interface:height")->value())));
      m_content.set<Component::Event>("cursor:clicked");
    }
  }
}

} // namespace Sosage::System
