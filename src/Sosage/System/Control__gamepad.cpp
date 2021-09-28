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

namespace Sosage::System
{

namespace C = Component;

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
        if (!contains (new_active_objects, active_object->value()))
          active_object->set (new_active_objects.front());
      }
      else
        set<C::String>("Interface:active_object", new_active_objects.front());
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
      if (!contains (new_active_objects, active_object->value()))
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

void Control::menu_gamepad()
{
  if (auto right = request<C::Boolean>("Switch:right"))
  {
    remove ("Switch:right");
    menu_sub_triggered(right ? RIGHT : LEFT);
  }

  Event_value stick = stick_left_right_up_down();
  if (stick != NONE)
    menu_sub_triggered(stick);

  std::string received_key = "";
  for (const std::string& key : {"move", "take", "inventory", "look"})
    if (receive("Action:" + key))
      received_key = key;

  if (received_key == "inventory")
    menu_sub_triggered (SOUTH);
  else if (received_key == "look")
    menu_sub_triggered (EAST);
}

void Control::menu_sub_triggered (const Event_value& key)
{
  const std::string& menu = get<C::String>("Game:current_menu")->value();
  bool settings = (menu == "Settings");

  auto active_item = request<C::String>("Interface:gamepad_active_menu_item");
  if (!active_item)
   return;

  if (key == UP_ARROW)
    menu_sub_switch_active_item (false);
  else if (key == DOWN_ARROW)
    menu_sub_switch_active_item (true);
  else if (key == LEFT)
  {
    if (settings)
    {
      if (!contains (active_item->value(), "_button"))
      {
        set<C::String>("Interface:active_menu_item", active_item->value() + "_left_arrow");
        emit ("Menu:clicked");
      }
    }
    else
      menu_sub_switch_active_item (false);
  }
  else if (key == RIGHT)
  {
    if (settings)
    {
      if (!contains (active_item->value(), "_button"))
      {
        set<C::String>("Interface:active_menu_item", active_item->value() + "_right_arrow");
        emit ("Menu:clicked");
      }
    }
    else
      menu_sub_switch_active_item (true);
  }
  else if (key == EAST)
  {
    if (settings && !contains (active_item->value(), "_button"))
    {
      set<C::String>("Interface:active_menu_item", active_item->value() + "_right_arrow");
      emit ("Menu:clicked");
    }
    else
    {
      set<C::String>("Interface:active_menu_item", active_item->value());
      emit ("Menu:clicked");
    }
  }
  else if (key == SOUTH)
  {
    emit("Game:escape");
    update_exit();
  }
}

void Control::menu_sub_switch_active_item (bool right)
{
  const std::string& id = get<C::String>("Game:current_menu")->value();
  auto menu = get<C::Menu>(id + ":menu");
  bool settings = (id == "Settings");

  auto active_item = get<C::String>("Interface:gamepad_active_menu_item");

  std::queue<C::Menu::Node> todo;
  todo.push (menu->root());
  std::vector<std::string> nodes;
  std::size_t nb_current = std::size_t(-1);
  while (!todo.empty())
  {
    C::Menu::Node current = todo.front();
    todo.pop();

    if (current.nb_children() < 2)
    {
      std::string entity = current.image()->entity();

      if (settings)
      {
        std::size_t pos = entity.find("_left_arrow");
        if (pos != std::string::npos)
         nodes.emplace_back(entity.begin(), entity.begin() + pos);
      }
      if (entity.find("_button") != std::string::npos)
        nodes.push_back (current.image()->entity());
      if (!nodes.empty() && nodes.back() == active_item->value())
        nb_current = nodes.size() - 1;
    }
    for (std::size_t i = 0; i < current.nb_children(); ++ i)
      todo.push (current[i]);
  }

  check (nb_current != std::size_t(-1), "Node " + active_item->value() + " not found in menu");
  if (right)
  {
    if (nb_current < nodes.size() - 1)
      active_item->set (nodes[nb_current + 1]);
    else
      active_item->set (nodes.front());
  }
  else
  {
    if (nb_current > 0)
      active_item->set (nodes[nb_current - 1]);
    else
      active_item->set (nodes.back());
  }

  set<C::String>("Interface:active_menu_item", active_item->value());
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
