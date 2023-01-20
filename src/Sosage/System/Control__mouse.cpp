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
#include <Sosage/Utils/conversions.h>

#include <queue>

namespace Sosage::System
{

namespace C = Component;

void Control::idle_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // If cursor below inventory threshold, go to inventory
  if (cursor->value().y() > Config::world_height - Config::inventory_active_zone / 5
      || (cursor->value().y() > Config::world_height - Config::inventory_active_zone
          && cursor->value().x() < get<C::Image>("Inventory_label_background", "image")->width()))
  {
    if (receive("Cursor", "clicked"))
      emit ("Cancel", "action");
    status()->push (IN_INVENTORY);
    return;
  }

  // Detect collision with clickable objets
  auto source = request<C::String>("Interface", "source_object");

  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    if (!request<C::String>(img->entity() , "name"))
      return false;
    // Can't combine with a goto
    if (source)
      if (request<C::Boolean>(img->entity() + "_goto", "right"))
        return false;
    return true;
  });

  if (collision != "")
    set<C::String>("Interface", "active_object", collision);
  else
    remove ("Interface", "active_object", true);

  if (receive("Cursor", "clicked"))
    idle_sub_click (collision);
}

void Control::idle_touchscreen()
{
  if (request<C::Signal>("Cursor", "clicked"))
    remove ("Player", "not_moved_yet", true);

  idle_sub_update_active_objects();

  if (receive("Cursor", "clicked"))
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
    if (auto active_objects = request<C::Vector<std::string>>("Interface", "active_objects"))
        collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
        {
          std::string id = img->entity();
          std::size_t pos = id.find("_label");
          if (pos != std::string::npos)
            id.resize(pos);

          if (!request<C::String>(id , "name"))
            return false;

          // Only collide with active objects
          return (contains(active_objects->value(), id));
        });

    std::size_t pos = collision.find("_label");
    if (pos != std::string::npos)
      collision.resize(pos);

    if (collision != "")
      set<C::String>("Interface", "active_object", collision);
    idle_sub_click (collision);
  }
}

void Control::idle_sub_click (const std::string& target)
{
  emit ("Cancel", "action");
  auto source = request<C::String>("Interface", "source_object");
  if (target != "")
  {
    // Source exists
    if (source)
    {
      std::string action_id = target + "_inventory_" + source->value();
      set<C::String>("Click", "target", target);

      if (auto action = request<C::Action>(action_id , "action"))
        set<C::Variable>("Character", "triggered_action", action);
      else
        set<C::Variable>("Character", "triggered_action", get<C::Action>("Default_inventory", "action"));
      emit ("Click", "play_sound");
      remove("Interface", "source_object");
    }
    // Object is path to another room
    else if (auto right = request<C::Boolean>(target + "_goto", "right"))
    {
      set_action(target + "_goto", "Default_goto");
      emit ("Click", "play_sound");
    }
    // Default
    else
    {
      set<C::String>("Interface", "action_choice_target", target);
      status()->push (ACTION_CHOICE);
      emit ("Click", "play_sound");
    }
    remove ("Interface", "active_object");
  }
  else
  {
    if (source)
      remove("Interface", "source_object");
    else
      emit("Cursor", "clicked"); // Logic handles this click
  }
}

void Control::action_choice_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // Detect collision with labels/buttons
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    if (contains(img->entity(), "Inventory"))
      return false;
    return (contains(img->entity(), "_label") || contains(img->entity(), "_button"));
  });

  if (collision != "")
    set<C::String>("Interface", "active_button", collision);
  else
    remove ("Interface", "active_button", true);

  if (receive("Cursor", "clicked"))
    action_choice_sub_click (collision);
}

void Control::action_choice_touchscreen()
{
  if (receive("Cursor", "clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // Detect collision with labels/buttons
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      if (contains(img->entity(), "Inventory"))
        return false;
      return (contains(img->entity(), "_label") || contains(img->entity(), "_button"));
    });

    action_choice_sub_click (collision);
  }
}

void Control::action_choice_sub_click (const std::string& id)
{
  status()->pop();
  remove ("Interface", "action_choice_target", true);
  remove ("Interface", "active_action", true);

  if (id == "")
    return;

  // Avoid clicking on outdated labels fading away
  if (endswith(id, "_old"))
    return;

  debug << "Click on " << id << std::endl;
  std::size_t pos = id.find("_button_");
  if (pos == std::string::npos)
    pos = id.find("_label");
  check (pos != std::string::npos, "Expected label or button, got " + id);

  std::string object_id (id.begin(), id.begin() + pos);

  pos = object_id.find_last_of('_');

  // Click on dying non-action label is very unlikely but can
  // happen, so just in case, check and exit
  if (pos == std::string::npos)
    return;

  std::string target (object_id.begin(), object_id.begin() + pos);
  std::string action (object_id.begin() + pos + 1, object_id.end());

  if (action == "inventory")
  {
    set<C::String>("Interface", "target_object", target);
    status()->push(OBJECT_CHOICE);
  }
  else if (action == "combine")
    set<C::String>("Interface", "source_object", target);
  else if (contains(Config::possible_actions, action))
    set_action (object_id, "Default_" + action);
  else
  {
    debug << "Action not found " << action << std::endl;
    return; // Click on outdated label
  }
  emit ("Click", "play_sound");
}

void Control::object_choice_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // Detect collision with clickable objets
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    std::string id = img->entity();
    if (!request<C::String>(id , "name"))
      return false;
    auto state = request<C::String>(id , "state");
    if (!state)
      return false;

    return startswith(state->value(), "inventory");
  });

  if (collision != "")
    set<C::String>("Interface", "active_object", collision);
  else
    remove ("Interface", "active_object", true);

  if (receive("Cursor", "clicked"))
  {
    // If cursor above inventory threshold, go back to idle
    if (cursor->value().y() < Config::world_height - Config::inventory_height)
    {
      status()->pop();
      remove ("Interface", "target_object");
      return;
    }

    object_choice_sub_click (collision);
  }
}

void Control::object_choice_touchscreen()
{
  if (receive("Cursor", "clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // Detect collision with clickable objets
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      std::string id = img->entity();
      if (!request<C::String>(id , "name"))
        return false;
      auto state = request<C::String>(id , "state");
      if (!state)
        return false;

      return startswith(state->value(), "inventory");
    });

    if (collision != "")
      set<C::String>("Interface", "active_object", collision);
    else
      remove ("Interface", "active_object", true);

    // If cursor above inventory threshold, go back to idle
    if (cursor->value().y() < Config::world_height - Config::inventory_height)
    {
      status()->pop();
      remove ("Interface", "target_object");
      return;
    }
    object_choice_sub_click (collision);
  }
}

void Control::object_choice_sub_click (const std::string& id)
{
  if (id == "")
    return;

  auto target = get<C::String>("Interface", "target_object");
  std::string action_id = target->value() + "_inventory_" + id;
  set<C::String>("Click", "target", target->value());
  set_action (action_id, "Default_inventory");
  remove ("Interface", "target_object");
  remove("Interface", "active_object");
  status()->pop();
  emit ("Click", "play_sound");
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
    if (!request<C::String>(id , "name"))
      return false;
    auto state = request<C::String>(id , "state");
    if (!state)
      return false;

    if (auto source = request<C::String>("Interface", "source_object"))
      if (source->value() == id)
        return false;

    return startswith(state->value(), "inventory");
  });

  if (collision != "")
    set<C::String>("Interface", "active_object", collision);
  else
    remove ("Interface", "active_object", true);

  if (receive("Cursor", "clicked"))
    inventory_sub_click (collision);
}

void Control::inventory_touchscreen()
{
  if (receive("Cursor", "clicked"))
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
      if (!request<C::String>(id , "name"))
        return false;
      auto state = request<C::String>(id , "state");
      if (!state)
        return false;

      if (auto source = request<C::String>("Interface", "source_object"))
        if (source->value() == id)
          return false;

      return startswith(state->value(), "inventory");
    });

    if (collision != "")
      set<C::String>("Interface", "active_object", collision);
    else
      remove ("Interface", "active_object", true);

    inventory_sub_click (collision);
  }
}

void Control::inventory_sub_click (const std::string& target)
{
  if (target == "")
    return;

  emit ("Click", "play_sound");
  emit ("Cancel", "action");
  auto source = request<C::String>("Interface", "source_object");

  if (source)
  {
    std::string action_id = target + "_inventory_" + source->value();
    set_action (action_id, "Default_inventory");
    remove("Interface", "source_object");
  }
  else // No target, no source, create actions for inventory object
  {
    set<C::String>("Interface", "action_choice_target", target);
    status()->pop();
    status()->push (INVENTORY_ACTION_CHOICE);
  }
  remove("Interface", "active_object");
}

void Control::window_mouse()
{
  if (receive("Cursor", "clicked"))
  {
    emit("Interface", "hide_window");
    status()->pop();
  }
}
void Control::window_touchscreen()
{
  if (receive("Cursor", "clicked"))
  {
    emit("Interface", "hide_window");
    status()->pop();
  }
}

void Control::code_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  auto code = get<C::Code>("Game", "code");
  auto window = get<C::Image>("Game", "window");

  bool collision = collides (cursor, window);
  if (collision)
  {
    auto position
      = get<C::Position>(window->entity() , "position");

    Point p = cursor->value() - Vector(position->value()) + Vector (0.5  * window->width(),
                                                                    0.5 * window->height());

    if (code->hover(p.X(), p.Y()))
      emit("Code", "hover");
  }

  if (receive("Cursor", "clicked"))
    code_sub_click(collision);
}

void Control::code_touchscreen()
{
  if (receive("Cursor", "clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);
    auto window = get<C::Image>("Game", "window");
    code_sub_click(collides(cursor, window));
  }
}

void Control::code_sub_click(bool collision)
{
  auto code = get<C::Code>("Game", "code");
  auto window = get<C::Image>("Game", "window");
  if (!collision)
  {
    emit("Interface", "hide_window");
    code->reset();
    status()->pop();
  }
  else
  {
    auto position
      = get<C::Position>(window->entity() , "position");

    auto cursor = get<C::Position>(CURSOR__POSITION);

    Point p = cursor->value() - Vector(position->value()) + Vector (0.5  * window->width(),
                                                                    0.5 * window->height());
    if (code->click(p.X(), p.Y()))
      emit ("code", "button_clicked");
  }
}

void Control::dialog_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // Detect collision with clickable objets
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
      return contains(img->entity(), "Dialog_choice_") && !contains(img->entity(), "background");
  });

  int choice = -1;
  if (collision != "")
  {
    choice = to_int(std::string(collision.begin() +
                                    std::ptrdiff_t(std::string("Dialog_choice_").size()),
                                    collision.end()));
    set<C::Int>("Interface", "active_dialog_item", choice);
  }
  else
    remove ("Interface", "active_dialog_item", true);

  if (receive("Cursor", "clicked") && choice != -1)
    dialog_sub_click ();
}

void Control::dialog_touchscreen()
{
  if (receive("Cursor", "clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // Detect collision with clickable objets
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      return contains(img->entity(), "Dialog_choice_") && !contains(img->entity(), "background");
    });

    int choice = -1;
    if (collision != "")
    {
      choice = to_int(std::string(collision.begin() +
                                      std::ptrdiff_t(std::string("Dialog_choice_").size()),
                                      collision.end()));
      set<C::Int>("Interface", "active_dialog_item", choice);
      dialog_sub_click ();
    }
    else
      remove ("Interface", "active_dialog_item", true);
  }
}

void Control::dialog_sub_click ()
{
  set<C::Int>("Dialog", "choice", value<C::Int>("Interface", "active_dialog_item"));
  emit("Dialog", "clean");
  remove ("Interface", "active_dialog_item", true);
  remove("Game", "current_dialog");

  status()->pop();
  emit ("Click", "play_sound");
}

void Control::menu_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // Detect collision with clickable objets
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    return contains(img->entity(), "_button") || contains(img->entity(), "_arrow");
  });

  if (collision != "")
    set<C::String>("Interface", "active_menu_item", collision);
  else
    remove ("Interface", "active_menu_item", true);

  if (receive("Cursor", "clicked") && collision != "")
    emit ("Menu", "clicked");
}

void Control::menu_touchscreen()
{
  if (receive("Cursor", "clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // Detect collision with clickable objets
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      return contains(img->entity(), "_button") || contains(img->entity(), "_arrow");
    });

    if (collision != "")
    {
      set<C::String>("Interface", "active_menu_item", collision);
      emit ("Menu", "clicked");
    }
  }
}

bool Control::collides (C::Position_handle cursor, C::Image_handle img)
{
  if (!img->on() || img->collision() == UNCLICKABLE)
    return false;

  auto position = get<C::Position>(img->entity() , "position");
  Point p = position->value();

  if (auto absol = C::cast<C::Absolute_position>(position))
    if (!absol->is_interface())
      p = p + Vector (-value<C::Absolute_position>(CAMERA__POSITION).x(), 0);

  Point screen_position = p - img->scale() * Vector(img->origin());
  int xmin = screen_position.X();
  int ymin = screen_position.Y();
  int xmax = xmin + int(img->scale() * (img->xmax() - img->xmin()));
  int ymax = ymin + int(img->scale() * (img->ymax() - img->ymin()));

  if (cursor->value().X() < xmin ||
      cursor->value().X() >= xmax ||
      cursor->value().Y() < ymin ||
      cursor->value().Y() >= ymax)
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
  SOSAGE_TIMER_START(System_Control__Collision_test);
  C::Image_handle out;
  for (const auto& e : components("image"))
    if (auto img = C::cast<C::Image>(e))
    {
      if (out && img->z() < out->z())
        continue;
      if (filter(img) & collides(cursor, img))
          out = img;
    }
  SOSAGE_TIMER_STOP(System_Control__Collision_test);
  return out ? out->entity() : "";
}

} // namespace Sosage::System
