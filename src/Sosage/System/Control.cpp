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

std::size_t Control::Hash_status_mode_pair::operator() (const Status_input_pair& p) const
{
  return std::hash<std::size_t>()(std::size_t(p.first))
      ^  std::hash<std::size_t>()(std::size_t(p.second));
}


Control::Control(Content& content)
  : Base (content)
  , m_latest_exit (0)
  , m_stick_on (false)
{
  INIT_DISPATCHER (IDLE, MOUSE, idle_mouse);
  INIT_DISPATCHER (IDLE, TOUCHSCREEN, idle_touchscreen);
  INIT_DISPATCHER (IDLE, GAMEPAD, idle_gamepad);
  INIT_DISPATCHER (ACTION_CHOICE, MOUSE, action_choice_mouse);
  INIT_DISPATCHER (ACTION_CHOICE, TOUCHSCREEN, action_choice_touchscreen);
  INIT_DISPATCHER (ACTION_CHOICE, GAMEPAD, action_choice_gamepad);
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
  INIT_DISPATCHER (DIALOG_CHOICE, MOUSE, dialog_mouse);
  INIT_DISPATCHER (DIALOG_CHOICE, TOUCHSCREEN, dialog_touchscreen);
  INIT_DISPATCHER (DIALOG_CHOICE, GAMEPAD, dialog_gamepad);
  INIT_DISPATCHER (IN_MENU, MOUSE, menu_mouse);
  INIT_DISPATCHER (IN_MENU, TOUCHSCREEN, menu_touchscreen);
  INIT_DISPATCHER (IN_MENU, GAMEPAD, menu_gamepad);
}

void Control::run()
{
  SOSAGE_TIMER_START(System_Control__run);
  SOSAGE_UPDATE_DBG_LOCATION("Control::run()");
  const Input_mode& new_mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
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
  SOSAGE_TIMER_STOP(System_Control__run);
}

void Control::init()
{
  m_mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
  m_status = status()->value();
}

void Control::update_exit()
{
  if (signal("Game", "escape"))
    emit("Game", "clear_notifications");

  if (status()->is (LOCKED))
  {
    if (receive("Game", "escape"))
    {
      if (request<C::Image>("Comment_0", "image"))
        emit("Game", "skip_dialog");
    }
    return;
  }
  if (status()->is (CUTSCENE))
  {
    double time = value<C::Double>(CLOCK__TIME);
    bool exit_message_exists = signal ("Skip_message", "exists");

    if (receive("Game", "escape") || receive("Game", "clear_notifications"))
    {
      if (exit_message_exists)
      {
        emit("Game", "skip_cutscene");
        receive("Skip_message", "exists");
      }
      else
      {
        emit("Skip_message", "exists");
        emit("Skip_message", "create");
      }
      m_latest_exit = time;
    }
    if (exit_message_exists && time - m_latest_exit >= Config::key_repeat_delay)
      receive("Skip_message", "exists");
  }
  else // status != CUTSCENE
  {
    if (receive("Game", "escape"))
    {
      if (status()->is (IN_MENU))
      {
        const std::string& menu = value<C::String>("Game", "current_menu");
        set<C::String>("Menu", "delete", menu);
        status()->pop();
      }
      else
      {
        set<C::String>("Menu", "create", "Exit");
        status()->push (IN_MENU);
      }
    }
  }
  if (receive("Show", "menu"))
  {
    set<C::String>("Menu", "create", value<C::String>("Game", "triggered_menu"));
    status()->push (IN_MENU);
  }
}

void Control::begin_status (const Status& s)
{
  m_stick_on = false;
  if (m_mode == GAMEPAD)
  {
    if (s == INVENTORY_ACTION_CHOICE)
    {
      status()->pop();
      m_status = status()->value();
    }
    else if (s == IN_INVENTORY || s == OBJECT_CHOICE)
    {
      auto inventory = get<C::Inventory>("Game", "inventory");
      if (auto source = request<C::String>("Interface", "source_object"))
      {
        for (std::size_t i = 0; i < inventory->size(); ++ i)
          if (inventory->get(i) == source->value())
          {
            if (i == inventory->size() - 1)
              set<C::String>("Interface", "active_object", inventory->get(i-1));
            else
              set<C::String>("Interface", "active_object", inventory->get(i+1));
            break;
          }
      }
      else
      {
        std::size_t idx = 0;
        if (auto previous = request<C::String>("Interface", "previous_active_inventory_object"))
        {
          for (std::size_t i = 0; i < inventory->size(); ++ i)
            if (inventory->get(i) == previous->value())
            {
              idx = i;
              break;
            }
          remove ("Interface", "previous_active_inventory_object");
        }
        set<C::String>("Interface", "active_object", inventory->get(idx));
      }
    }
    else if (s == IN_CODE)
    {
      get<C::Code>("Game", "code")->hover();
      emit("Code", "hover");
    }
    else if (s == DIALOG_CHOICE)
      set<C::Int>("Interface", "active_dialog_item", 0);
  }
}

void Control::end_status (const Status& s)
{
  if (s == IDLE)
  {
    if (m_mode == GAMEPAD)
      if (auto active = request<C::String>("Interface", "active_object"))
        set<C::String>("Interface", "previous_active_object", active->value());
    remove ("Interface", "active_object", true);
    remove ("Interface", "active_objects", true);
  }
}

void Control::set_action (const std::string& id, const std::string& default_id)
{
  debug << "Set action to " << id << " (fallback to " << default_id << ")" << std::endl;
  if (auto action = request<C::Action>(id , "action"))
    set<C::Variable>("Character", "triggered_action", action);
  else
    set<C::Variable>("Character", "triggered_action", get<C::Action>(default_id , "action"));
}

} // namespace Sosage::System
