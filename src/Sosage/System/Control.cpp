/*
  [src/Sosage/System/Control.cpp]
  Handles how users control the interface.

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

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Code.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Menu.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Control.h>

#include <queue>

#define INIT_DISPATCHER(s, m, f) \
  m_dispatcher.insert (std::make_pair(std::make_pair(s, m), std::bind(&Control::f, this)))


namespace Sosage::System
{

namespace C = Component;

Control::Control(Content& content)
  : Base (content)
  , m_status(LOCKED)
  , m_latest_exit (0)
  , m_stick_on (false)
{
  INIT_DISPATCHER (IDLE, MOUSE, idle_mouse);
  INIT_DISPATCHER (IDLE, TOUCHSCREEN, idle_touchscreen);
  INIT_DISPATCHER (IDLE, GAMEPAD, idle_gamepad);
  INIT_DISPATCHER (ACTION_CHOICE, MOUSE, action_choice_mouse);
  INIT_DISPATCHER (ACTION_CHOICE, TOUCHSCREEN, action_choice_touchscreen);
  INIT_DISPATCHER (OBJECT_CHOICE, MOUSE, object_choice_mouse);
  INIT_DISPATCHER (OBJECT_CHOICE, TOUCHSCREEN, object_choice_touchscreen);
  INIT_DISPATCHER (OBJECT_CHOICE, GAMEPAD, object_choice_gamepad);
  INIT_DISPATCHER (IN_INVENTORY, MOUSE, inventory_mouse);
  INIT_DISPATCHER (IN_INVENTORY, TOUCHSCREEN, inventory_touchscreen);
  INIT_DISPATCHER (IN_INVENTORY, GAMEPAD, inventory_gamepad);
  INIT_DISPATCHER (INVENTORY_ACTION_CHOICE, MOUSE, action_choice_mouse);
  INIT_DISPATCHER (INVENTORY_ACTION_CHOICE, TOUCHSCREEN, action_choice_touchscreen);
  INIT_DISPATCHER (IN_WINDOW, MOUSE, window_mouse);
  INIT_DISPATCHER (IN_WINDOW, TOUCHSCREEN, window_touchscreen);
  INIT_DISPATCHER (IN_WINDOW, GAMEPAD, window_gamepad);
  INIT_DISPATCHER (IN_CODE, MOUSE, code_mouse);
  INIT_DISPATCHER (IN_CODE, TOUCHSCREEN, code_touchscreen);
  INIT_DISPATCHER (IN_CODE, GAMEPAD, code_gamepad);
  INIT_DISPATCHER (IN_MENU, MOUSE, menu_mouse);
  INIT_DISPATCHER (IN_MENU, TOUCHSCREEN, menu_touchscreen);
  INIT_DISPATCHER (IN_MENU, GAMEPAD, menu_gamepad);
}

void Control::run()
{
  const Input_mode& new_mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();
  const Status& new_status = status()->value();

  if (new_status != m_status || new_mode != m_mode)
  {
    end_status(m_status);
    m_status = new_status;
    m_mode = new_mode;
    begin_status(m_status);
  }

  update_exit();

  auto key = std::make_pair (m_status, m_mode);
  auto iter = m_dispatcher.find(key);
  if (iter != m_dispatcher.end())
    (iter->second)();
}

void Control::update_exit()
{
  if (status()->is (LOCKED))
  {
    receive("Game:escape");
    return;
  }

  if (status()->is (CUTSCENE))
  {
    double time = get<C::Double>(CLOCK__TIME)->value();
    bool exit_message_exists = (request<C::Image>("Exit_message:image") != nullptr);

    if (receive("Game:escape"))
    {
      if (time - m_latest_exit < Config::key_repeat_delay)
      {
        emit("Game:skip_cutscene");
        if (exit_message_exists)
          emit("Exit_message:remove");
#if 0
        {
          remove("Exit_message:image");
          remove("Exit_message:position");
          remove("Exit_message_back:image");
          remove("Exit_message_back:position");
        }
#endif
        return;
      }
      m_latest_exit = time;
    }

    if (time - m_latest_exit < Config::key_repeat_delay)
    {
      if (!exit_message_exists)
        emit("Exit_message:create");
#if 0
      {
        auto interface_font = get<C::Font> ("Interface:font");

        auto img
            = set<C::Image>("Exit_message:image", interface_font, "FFFFFF",
                            get<C::String>("Skip_cutscene:text")->value());
        img->z() += 10;
        img->set_scale(0.5);
        img->set_relative_origin (1, 1);

        auto img_back
            = set<C::Image>("Exit_message_back:image", 0.5 * img->width() + 10, 0.5 * img->height() + 10);
        img_back->z() = img->z() - 1;
        img_back->set_relative_origin (1, 1);

        int window_width = Config::world_width;
        int window_height = Config::world_height;
        set<C::Absolute_position>("Exit_message:position", Point (window_width - 5,
                                                         window_height - 5));
        set<C::Absolute_position>("Exit_message_back:position", Point (window_width,
                                                              window_height));
      }
#endif
    }
    else
    {
      if (exit_message_exists)
        emit("Exit_message:remove");
    }
  }
  else // status != CUTSCENE
  {
    if (receive("Game:escape"))
    {
      if (status()->is (IN_MENU))
      {
        const std::string& menu = get<C::String>("Game:current_menu")->value();
        if (menu == "End")
          emit("Game:exit");
        else
        {
          set<C::String>("Menu:delete", menu);
          status()->pop();
        }
      }
      else
      {
        set<C::String>("Menu:create", "Exit");
        status()->push (IN_MENU);
      }
    }
  }
}

void Control::begin_status (const Status& s)
{
  m_stick_on = false;
  if (m_mode == GAMEPAD)
  {
    if (s == ACTION_CHOICE || s == INVENTORY_ACTION_CHOICE)
    {
      status()->pop();
      m_status = status()->value();
    }
    else if (s == IN_INVENTORY || s == OBJECT_CHOICE)
    {
      if (auto source = request<C::String>("Interface:source_object"))
      {
        auto inventory = get<C::Inventory>("Game:inventory");
        for (std::size_t i = 0; i < inventory->size(); ++ i)
          if (inventory->get(i) == source->value())
          {
            if (i == inventory->size() - 1)
              set<C::String>("Interface:active_object", inventory->get(i-1));
            else
              set<C::String>("Interface:active_object", inventory->get(i+1));
            break;
          }
      }
      else
        set<C::String>("Interface:active_object", get<C::Inventory>("Game:inventory")->get(0));
    }
    else if (s == IN_CODE)
    {
      get<C::Code>("Game:code")->hover();
      emit("Code:hover");
    }
  }
}

void Control::end_status (const Status& s)
{
  if (s == IDLE)
  {
    remove ("Interface:active_object", true);
    remove ("Interface:active_objects", true);
  }
}

void Control::set_action (const std::string& id, const std::string& default_id)
{
  debug ("Set action to " + id + " (fallback to " + default_id + ")");
  if (auto action = request<C::Action>(id + ":action"))
    set<C::Variable>("Character:action", action);
  else
    set<C::Variable>("Character:action", get<C::Action>(default_id + ":action"));
}

} // namespace Sosage::System
