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
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Control.h>

#define INIT_DISPATCHER(s, m, f) \
  m_dispatcher.insert (std::make_pair(std::make_pair(s, m), std::bind(&Control::f, this)))


namespace Sosage::System
{

namespace C = Component;

Control::Control(Content& content)
  : Base (content)
  , m_status(LOCKED)
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

  auto key = std::make_pair (m_status, m_mode);
  auto iter = m_dispatcher.find(key);
  if (iter != m_dispatcher.end())
    (iter->second)();
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

void Control::idle_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // If cursor below inventory threshold, go to inventory
  if (cursor->value().y() > Config::world_height - Config::inventory_active_zone)
  {
    status()->push (IN_INVENTORY);
    return;
  }

  // Detect collision with clickable objets
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  { return bool(request<C::String>(img->entity() + ":name")); });

  if (collision != "")
    set<C::String>("Interface:active_object", collision);
  else
    remove ("Interface:active_object", true);

  if (receive("Cursor:clicked"))
    idle_sub_click (collision);
}

void Control::idle_touchscreen()
{
  idle_sub_update_active_objects();

  if (receive("Cursor:clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // If cursor below inventory threshold, go to inventory
    if (cursor->value().y() > Config::world_height - Config::inventory_active_zone)
    {
      status()->push (IN_INVENTORY);
      return;
    }

    // Detect collision with clickable objets
    std::string collision = "";
    if (auto active_objects = request<C::Vector<std::string>>("Interface:active_objects"))
        collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
        {
          std::string id = img->entity();
          std::size_t pos = id.find("_label");
          if (pos != std::string::npos)
            id.resize(pos);

          if (!request<C::String>(id + ":name"))
            return false;

          // Only collide with active objects
          return (std::find(active_objects->value().begin(),
                            active_objects->value().end(), id)
                  != active_objects->value().end());
        });

    std::size_t pos = collision.find("_label");
    if (pos != std::string::npos)
      collision.resize(pos);

    idle_sub_click (collision);
  }
}

void Control::idle_gamepad()
{
  idle_sub_update_active_objects();

  if (auto right = request<C::Boolean>("Switch:right"))
  {
    idle_sub_switch_active_object(right->value());
    remove ("Switch:right");
  }

  std::string received_key = "";
  for (const std::string& key : {"move", "take", "inventory", "look"})
    if (receive("Action:" + key))
      received_key = key;

  if (received_key != "")
    idle_sub_triggered (received_key);
}

void Control::idle_sub_click (const std::string& target)
{
  emit ("Click:play_sound");
  auto source = request<C::String>("Interface:source_object");
  if (target != "")
  {
    // Source exists
    if (source)
    {
      std::string action_id = target + "_inventory_" + source->value();
      if (auto action = request<C::Action>(action_id + ":action"))
        set<C::Variable>("Character:action", action);
      else
        set<C::Variable>("Character:action", get<C::Action>("Default_inventory:action"));
      remove("Interface:source_object");
    }
    // Object is path to another room
    else if (auto right = request<C::Boolean>(target + "_goto:right"))
      set_action(target + "_goto", "Default_goto");
    // Default
    else
    {
      set<C::String>("Interface:action_choice_target", target);
      status()->push (ACTION_CHOICE);
    }
    remove ("Interface:active_object");
  }
  else
  {
    if (source)
      remove("Interface:source_object");
    else
      emit("Cursor:clicked"); // Logic handles this click
  }
}

void Control::idle_sub_update_active_objects()
{
  std::vector<std::string> new_active_objects = detect_active_objects();

  // No active objects
  if (new_active_objects.empty())
  {
    remove ("Interface:active_objects", true);
    remove ("Interface:active_object", true);
  }
  // Active objects changed
  else if (auto active_objects = request<C::Vector<std::string>>("Interface:active_objects"))
  {
    if (new_active_objects != active_objects->value())
    {
      if (auto active_object = request<C::String>("Interface:active_object"))
      {
        // If active object is not anymore in list of active objects, change it
        if (std::find(new_active_objects.begin(), new_active_objects.end(), active_object->value())
            == new_active_objects.end())
          active_object->set (new_active_objects.front());
      }
      else
        active_object->set (new_active_objects.front());
      active_objects->set (new_active_objects);
    }
  }
  // New active objects
  else
  {
    set<C::Vector<std::string>>("Interface:active_objects", new_active_objects);
    if (auto active_object = request<C::String>("Interface:active_object"))
    {
      // If active object is not anymore in list of active objects, change it
      if (std::find(new_active_objects.begin(), new_active_objects.end(), active_object->value())
          == new_active_objects.end())
        active_object->set (new_active_objects.front());
    }
    else
    {
      set<C::String>("Interface:active_object", new_active_objects.front());
    }
  }
}

void Control::idle_sub_switch_active_object (bool right)
{
  auto active_objects = request<C::Vector<std::string>>("Interface:active_objects");
  if (!active_objects)
    return;
  if (active_objects->value().size() < 2)
    return;

  auto active_object = get<C::String>("Interface:active_object");
  for (std::size_t i = 0; i < active_objects->value().size(); ++ i)
    if (active_objects->value()[i] == active_object->value())
    {
      active_object->set (right ? active_objects->value()[(i + 1) % active_objects->value().size()]
                          : active_objects->value()[(i + active_objects->value().size() - 1) % active_objects->value().size()]);
      return;
    }
}

void Control::idle_sub_triggered (const std::string& key)
{
  auto active_object = request<C::String>("Interface:active_object");

  if (!active_object)
  {
    if (key == "inventory")
    {
      status()->push (IN_INVENTORY);
      emit("Click:play_sound");
    }
    return;
  }

  if (auto right = request<C::Boolean>(active_object->value() + "_goto:right"))
  {
    if (key == "look")
    {
      set_action (active_object->value() + "_goto", "Default_goto");
      emit("Click:play_sound");
    }
    else if (key == "inventory")
    {
      status()->push (IN_INVENTORY);
      emit("Click:play_sound");
    }
    return;
  }

  if (key == "inventory")
  {
    set<C::String>("Interface:target_object", active_object->value());
    status()->push (OBJECT_CHOICE);
    emit("Click:play_sound");
    return;
  }
  set_action (active_object->value() + "_" + key, "Default_" + key);
  emit("Click:play_sound");
}


void Control::action_choice_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // Detect collision with labels/buttons
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    return (contains(img->entity(), "_label") || contains(img->entity(), "_button"));
  });

  if (collision != "")
    set<C::String>("Interface:active_button", collision);
  else
    remove ("Interface:active_button", true);

  if (receive("Cursor:clicked"))
    action_choice_sub_click (collision);
}

void Control::action_choice_touchscreen()
{
  if (receive("Cursor:clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // Detect collision with labels/buttons
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      return (contains(img->entity(), "_label") || contains(img->entity(), "_button"));
    });

    action_choice_sub_click (collision);
  }
}

void Control::action_choice_sub_click (const std::string& id)
{
  status()->pop();
  remove ("Interface:action_choice_target", true);
  remove ("Interface:active_action", true);

  if (id == "")
    return;

  std::size_t pos = id.find("_button_");
  if (pos == std::string::npos)
    pos = id.find("_label");
  check (pos != std::string::npos, "Expected label or button, got " + id);

  std::string object_id (id.begin(), id.begin() + pos);

  pos = object_id.find_last_of('_');
  check(pos != std::string::npos, "Ill-formed action entity " + object_id);
  std::string target (object_id.begin(), object_id.begin() + pos);
  std::string action (object_id.begin() + pos + 1, object_id.end());

  if (action == "inventory")
  {
    set<C::String>("Interface:target_object", target);
    status()->push(OBJECT_CHOICE);
  }
  else if (action == "combine")
    set<C::String>("Interface:source_object", target);
  else
    set_action (object_id, "Default_" + action);
  emit ("Click:play_sound");
}

void Control::object_choice_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // Detect collision with clickable objets
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    std::string id = img->entity();
    if (!request<C::String>(id + ":name"))
      return false;
    auto state = request<C::String>(id + ":state");
    if (!state)
      return false;

    return (state->value() == "inventory");
  });

  if (collision != "")
    set<C::String>("Interface:active_object", collision);
  else
    remove ("Interface:active_object", true);

  if (receive("Cursor:clicked"))
  {
    // If cursor above inventory threshold, go back to idle
    if (cursor->value().y() < Config::world_height - Config::inventory_height)
    {
      status()->pop();
      remove ("Interface:target_object");
      return;
    }

    object_choice_sub_click (collision);
  }
}

void Control::object_choice_touchscreen()
{
  if (receive("Cursor:clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // Detect collision with clickable objets
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      std::string id = img->entity();
      if (!request<C::String>(id + ":name"))
        return false;
      auto state = request<C::String>(id + ":state");
      if (!state)
        return false;

      return (state->value() == "inventory");
    });

    if (collision != "")
      set<C::String>("Interface:active_object", collision);
    else
      remove ("Interface:active_object", true);

    // If cursor above inventory threshold, go back to idle
    if (cursor->value().y() < Config::world_height - Config::inventory_height)
    {
      status()->pop();
      remove ("Interface:target_object");
      return;
    }
    object_choice_sub_click (collision);
  }
}

void Control::object_choice_gamepad()
{
  if (auto right = request<C::Boolean>("Switch:right"))
  {
    inventory_sub_switch_active_object(right->value());
    remove ("Switch:right");
  }

  Event_value stick = stick_left_right();
  if (stick != NONE)
    inventory_sub_switch_active_object(stick == RIGHT);

  std::string received_key = "";
  for (const std::string& key : {"move", "take", "inventory", "look"})
    if (receive("Action:" + key))
      received_key = key;

  if (received_key != "")
    object_choice_sub_triggered (received_key);
}

void Control::object_choice_sub_click (const std::string& id)
{
  if (id == "")
    return;

  auto target = get<C::String>("Interface:target_object");
  std::string action_id = target->value() + "_inventory_" + id;
  set_action (action_id, "Default_inventory");
  remove ("Interface:target_object");
  remove("Interface:active_object");
  status()->pop();
  emit ("Click:play_sound");
}

void Control::object_choice_sub_triggered (const std::string& key)
{
  auto active_object = request<C::String>("Interface:active_object");

  if (key == "look")
  {
    if (auto target = request<C::String>("Interface:target_object"))
    {
      std::string action_id = target->value() + "_inventory_" + active_object->value();
      set_action (action_id, "Default_inventory");
      remove("Interface:target_object");
    }
    else
    {
      auto source = get<C::String>("Interface:source_object");
      std::string action_id = active_object->value() + "_inventory_" + source->value();
      set_action (action_id, "Default_inventory");
      remove("Interface:source_object");
    }
    remove("Interface:active_object");
    status()->pop();
    emit("Click:play_sound");
  }
  else if (key == "inventory")
  {
    status()->pop();
    remove("Interface:target_object", true);
    remove("Interface:source_object", true);
    remove("Interface:active_object");
    emit("Click:play_sound");
  }
}

void Control::inventory_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // If cursor above inventory threshold, go back to idle
  if (cursor->value().y() < Config::world_height - Config::inventory_height)
  {
    status()->pop();
    return;
  }

  // Detect collision with clickable objets
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    std::string id = img->entity();
    if (!request<C::String>(id + ":name"))
      return false;
    auto state = request<C::String>(id + ":state");
    if (!state)
      return false;

    if (auto source = request<C::String>("Interface:source_object"))
      if (source->value() == id)
        return false;

    return (state->value() == "inventory");
  });

  if (collision != "")
    set<C::String>("Interface:active_object", collision);
  else
    remove ("Interface:active_object", true);

  if (receive("Cursor:clicked"))
    inventory_sub_click (collision);
}

void Control::inventory_touchscreen()
{
  if (receive("Cursor:clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // If cursor above inventory threshold, go back to idle
    if (cursor->value().y() < Config::world_height - Config::inventory_height)
    {
      status()->pop();
      return;
    }

    // Detect collision with clickable objets
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      std::string id = img->entity();
      if (!request<C::String>(id + ":name"))
        return false;
      auto state = request<C::String>(id + ":state");
      if (!state)
        return false;

      if (auto source = request<C::String>("Interface:source_object"))
        if (source->value() == id)
          return false;

      return (state->value() == "inventory");
    });

    if (collision != "")
      set<C::String>("Interface:active_object", collision);
    else
      remove ("Interface:active_object", true);

    inventory_sub_click (collision);
  }
}

void Control::inventory_gamepad()
{
  if (auto right = request<C::Boolean>("Switch:right"))
  {
    inventory_sub_switch_active_object(right->value());
    remove ("Switch:right");
  }

  Event_value stick = stick_left_right();
  if (stick != NONE)
    inventory_sub_switch_active_object(stick == RIGHT);

  std::string received_key = "";
  for (const std::string& key : {"move", "take", "inventory", "look"})
    if (receive("Action:" + key))
      received_key = key;

  if (received_key != "")
    inventory_sub_triggered (received_key);
}

void Control::inventory_sub_click (const std::string& target)
{
  if (target == "")
    return;

  emit ("Click:play_sound");
  auto source = request<C::String>("Interface:source_object");

  if (source)
  {
    std::string action_id = target + "_inventory_" + source->value();
    set_action (action_id, "Default_inventory");
    remove("Interface:source_object");
  }
  else // No target, no source, create actions for inventory object
  {
    set<C::String>("Interface:action_choice_target", target);
    status()->pop();
    status()->push (INVENTORY_ACTION_CHOICE);
  }
  remove("Interface:active_object");
}

void Control::inventory_sub_switch_active_object (bool right)
{
  auto inventory = get<C::Inventory>("Game:inventory");
  auto active_object = get<C::String>("Interface:active_object");
  std::string source_object = "";
  if (auto s = request<C::String>("Interface:source_object"))
    source_object = s->value();

  for (std::size_t i = 0; i < inventory->size(); ++ i)
    if (inventory->get(i) == active_object->value())
    {
      int diff = 0;
      if (right && i < inventory->size() - 1)
        diff = 1;
      else if (!right && i > 0)
        diff = -1;

      if (inventory->get(i+diff) == source_object)
      {
        diff = 0;
        if (right && i < inventory->size() - 2)
          diff = 2;
        else if (!right && i > 1)
          diff = -2;
      }
      active_object->set (inventory->get(i+diff));
      break;
    }
}

void Control::inventory_sub_triggered (const std::string& key)
{
  auto active_object = request<C::String>("Interface:active_object");

  if (key == "look")
  {
    std::string action_id = active_object->value() + "_look";
    set_action (action_id, "Default_look");
    status()->pop();
    remove ("Interface:active_object");
  }
  else if (key == "move") // use
  {
    std::string action_id = active_object->value() + "_use";
    set_action (action_id, "Default_use");
    status()->pop();
    remove ("Interface:active_object");
  }
  else if (key == "take") // combine
  {
    if (get<C::Inventory>("Game:inventory")->size() > 1)
    {
      status()->pop();
      status()->push(OBJECT_CHOICE);
      set<C::String>("Interface:source_object", active_object->value());
    }
  }
  else // if (action == "inventory")
  {
    status()->pop();
    remove ("Interface:active_object");
  }

  emit("Click:play_sound");
}

void Control::window_mouse()
{
  if (receive("Cursor:clicked"))
  {
    auto window = get<C::Image>("Game:window");
    window->on() = false;
    status()->pop();
  }
}
void Control::window_touchscreen()
{
  if (receive("Cursor:clicked"))
  {
    auto window = get<C::Image>("Game:window");
    window->on() = false;
    status()->pop();
  }
}

void Control::window_gamepad()
{
  std::string received_key = "";
  for (const std::string& key : {"move", "take", "inventory", "look"})
    if (receive("Action:" + key))
      received_key = key;

  if (received_key == "look" || received_key == "inventory")
  {
    auto window = get<C::Image>("Game:window");
    window->on() = false;
    status()->pop();
  }
}

void Control::code_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");

  bool collision = collides (cursor, window);
  if (collision)
  {
    auto position
      = get<C::Position>(window->entity() + ":position");

    Point p = cursor->value() - Vector(position->value()) + Vector (0.5  * window->width(),
                                                                    0.5 * window->height());

    if (code->hover(p.X(), p.Y()))
      emit("Code:hover");
  }

  if (receive("Cursor:clicked"))
    code_sub_click(collision);
}

void Control::code_touchscreen()
{
  if (receive("Cursor:clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);
    auto window = get<C::Image>("Game:window");
    code_sub_click(collides(cursor, window));
  }
}

void Control::code_gamepad()
{
  Vector direction = stick_direction();
  if (direction != Vector(0,0))
  {
    get<C::Code>("Game:code")->move(direction.x(), direction.y());
    emit("Code:hover");
  }

  std::string received_key = "";
  for (const std::string& key : {"move", "take", "inventory", "look"})
    if (receive("Action:" + key))
      received_key = key;

  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");
  if (received_key == "inventory")
  {
    window->on() = false;
    code->reset();
    remove("Code_hover:image");
    status()->pop();
  }
  else if (received_key == "look")
    if (code->click())
      emit ("code:button_clicked");
}

void Control::code_sub_click(bool collision)
{
  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");
  if (!collision)
  {
    window->on() = false;
    code->reset();
    status()->pop();
  }
  else
  {
    auto position
      = get<C::Position>(window->entity() + ":position");

    auto cursor = get<C::Position>(CURSOR__POSITION);

    Point p = cursor->value() - Vector(position->value()) + Vector (0.5  * window->width(),
                                                                    0.5 * window->height());
    if (code->click(p.X(), p.Y()))
      emit ("code:button_clicked");
  }
}

bool Control::collides (C::Position_handle cursor, C::Image_handle img)
{
  if (!img->on() || img->collision() == UNCLICKABLE)
    return false;

  auto position = get<C::Position>(img->entity() + ":position");
  Point p = position->value();

  if (auto absol = C::cast<C::Absolute_position>(position))
    if (!absol->absolute())
      p = p + Vector (-get<C::Absolute_position>(CAMERA__POSITION)->value().x(), 0);

  Point screen_position = p - img->core().scaling * Vector(img->origin());
  int xmin = screen_position.X();
  int ymin = screen_position.Y();
  int xmax = xmin + int(img->core().scaling * (img->xmax() - img->xmin()));
  int ymax = ymin + int(img->core().scaling * (img->ymax() - img->ymin()));

  if (cursor->value().x() < xmin ||
      cursor->value().x() >= xmax ||
      cursor->value().y() < ymin ||
      cursor->value().y() >= ymax)
    return false;

  if (img->collision() == PIXEL_PERFECT)
  {
    int x_in_image = cursor->value().X() - xmin;
    int y_in_image = cursor->value().Y() - ymin;

    if (!img->is_target_inside (x_in_image, y_in_image))
      return false;
  }

  return true;
}

std::string Control::first_collision
(C::Position_handle cursor, const std::function<bool(C::Image_handle)>& filter)
{
  C::Image_handle out;
  for (const auto& e : m_content)
    if (auto img = C::cast<C::Image>(e))
      if (filter(img) & collides(cursor, img))
        if (!out || img->z() > out->z())
          out = img;
  return out ? out->entity() : "";
}

void Control::set_action (const std::string& id, const std::string& default_id)
{
  debug ("Set action to " + id + " (fallback to " + default_id + ")");
  if (auto action = request<C::Action>(id + ":action"))
    set<C::Variable>("Character:action", action);
  else
    set<C::Variable>("Character:action", get<C::Action>(default_id + ":action"));
}

std::vector<std::string> Control::detect_active_objects()
{
  const std::string& id = get<C::String>("Player:name")->value();
  auto position = get<C::Position>(id + "_body:position");

  std::unordered_set<std::string> active_objects;
  if (auto a = request<C::Vector<std::string>>("Interface:active_objects"))
    active_objects = std::unordered_set<std::string>(a->value().begin(), a->value().end());

  // Find objects with labels close to player
  std::vector<std::string> out;
  for (const auto& e : m_content)
    if (auto label = C::cast<C::Absolute_position>(e))
      if (label->component() == "label")
      {
        auto pos = get<C::Position>(label->entity() + ":view");

        double dx = std::abs(position->value().x() - pos->value().x());
        double dy = std::abs(position->value().y() - pos->value().y());

        // Object out of reach
        if (dx > Config::object_reach_x + Config::object_reach_hysteresis ||
            dy > Config::object_reach_y + Config::object_reach_hysteresis)
          continue;

        // Inactive object
        if (!request<C::Image>(label->entity() + ":image"))
          continue;

        // Inventory objet
        if (get<C::String>(label->entity() + ":state")->value() == "inventory")
          continue;

        // Object in reach
        if (dx <= Config::object_reach_x && dy <= Config::object_reach_y)
          out.emplace_back (label->entity());

        // Object in hysteresis range
        else if (contains(active_objects, label->entity()))
          out.emplace_back (label->entity());
      }

  // Sort by X position
  std::sort (out.begin(), out.end(),
             [&](const std::string& a, const std::string& b) -> bool
  {
    auto pos_a = get<C::Position>(a + ":label");
    auto pos_b = get<C::Position>(b + ":label");
    return pos_a->value().x() < pos_b->value().x();
  });

  return out;
}

Event_value Control::stick_left_right()
{
  if (receive("Stick:moved"))
  {
    if (m_stick_on)
    {
      if (get<C::Simple<Vector>>(STICK__DIRECTION)->value() == Vector(0,0))
        m_stick_on = false;
      return NONE;
    }

    m_stick_on = true;

    double x = get<C::Simple<Vector>>(STICK__DIRECTION)->value().x();
    if (x == 0)
      return NONE;
    return (x > 0 ? RIGHT : LEFT);
  }
  return NONE;
}

Event_value Control::stick_left_right_up_down()
{
  if (receive("Stick:moved"))
  {
    if (m_stick_on)
    {
      if (get<C::Simple<Vector>>(STICK__DIRECTION)->value() == Vector(0,0))
        m_stick_on = false;
      return NONE;
    }

    m_stick_on = true;

    double x = get<C::Simple<Vector>>(STICK__DIRECTION)->value().x();
    double y = get<C::Simple<Vector>>(STICK__DIRECTION)->value().y();
    if (std::abs(x) > std::abs(y))
      return (x > 0 ? RIGHT : LEFT);
    // else
    return (y > 0 ? DOWN_ARROW : UP_ARROW);
  }
  return NONE;
}

Vector Control::stick_direction()
{
  if (receive("Stick:moved"))
  {
    if (m_stick_on)
    {
      if (get<C::Simple<Vector>>(STICK__DIRECTION)->value() == Vector(0,0))
        m_stick_on = false;
      return Vector(0,0);
    }
    m_stick_on = true;
    return get<C::Simple<Vector>>(STICK__DIRECTION)->value();
  }
  return Vector(0,0);
}


} // namespace Sosage::System
