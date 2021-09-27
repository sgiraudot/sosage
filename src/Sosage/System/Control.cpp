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
{
  INIT_DISPATCHER (IDLE, MOUSE, idle_mouse);
  INIT_DISPATCHER (IDLE, TOUCHSCREEN, idle_touchscreen);
  INIT_DISPATCHER (IDLE, GAMEPAD, idle_gamepad);
  INIT_DISPATCHER (ACTION_CHOICE, MOUSE, action_choice_mouse);
  INIT_DISPATCHER (ACTION_CHOICE, TOUCHSCREEN, action_choice_touchscreen);
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
  if (m_mode == GAMEPAD && s == ACTION_CHOICE)
  {
    status()->pop();
    m_status = status()->value();
  }

}

void Control::end_status (const Status& s)
{
  if (s == IDLE)
  {
    remove ("Interface:active_object", true);
    remove ("Interface:active_objects", true);
  }
  else if (s == ACTION_CHOICE)
  {
    remove ("Interface:action_choice_target", true);
    remove ("Interface:active_action", true);
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
    // Detect collision with clickable objets
    auto cursor = get<C::Position>(CURSOR__POSITION);
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      std::string id = img->entity();
      std::size_t pos = id.find("_label");
      if (pos != std::string::npos)
        id.resize(pos);

      if (!request<C::String>(id + ":name"))
        return false;

      // Only collide with active objects
      const std::vector<std::string>& active_objects
          = get<C::Vector<std::string>>("Interface:active_objects")->value();
      return (std::find(active_objects.begin(), active_objects.end(), id)
              != active_objects.end());
    });

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
    std::cerr << "Delete active objects" << std::endl;
    remove ("Interface:active_objects", true);
    remove ("Interface:active_object", true);
  }
  // Active objects changed
  else if (auto active_objects = request<C::Vector<std::string>>("Interface:active_objects"))
  {
    if (new_active_objects != active_objects->value())
    {
      auto active_object = get<C::String>("Interface:active_object");
      // If active object is not anymore in list of active objects, change it
      if (std::find(new_active_objects.begin(), new_active_objects.end(), active_object->value())
          == new_active_objects.end())
        active_object->set (new_active_objects.front());
      active_objects->set (new_active_objects);
    }
  }
  // New active objects
  else
  {
    std::cerr << "New active objects" << std::endl;
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
    return (img->entity().find("_label") != std::string::npos) ||
        (img->entity().find("_button") != std::string::npos);
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
      return (img->entity().find("_label") != std::string::npos) ||
          (img->entity().find("_button") != std::string::npos);
    });

    action_choice_sub_click (collision);
  }
}

void Control::action_choice_sub_click (const std::string& id)
{
  status()->pop();

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
  else
    set_action (object_id, "Default_" + action);
  emit ("Click:play_sound");
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
        else if (active_objects.find(label->entity()) != active_objects.end())
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


} // namespace Sosage::System
