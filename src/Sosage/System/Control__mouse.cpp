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
  auto source = request<C::String>("Interface:source_object");

  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    if (!request<C::String>(img->entity() + ":name"))
      return false;
    // Can't combine with a goto
    if (source)
      if (request<C::Boolean>(img->entity() + "_goto:right"))
        return false;
    return true;
  });

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
          return (contains(active_objects->value(), id));
        });

    std::size_t pos = collision.find("_label");
    if (pos != std::string::npos)
      collision.resize(pos);

    idle_sub_click (collision);
  }
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

void Control::menu_mouse()
{
  auto cursor = get<C::Position>(CURSOR__POSITION);

  // Detect collision with clickable objets
  std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
  {
    return contains(img->entity(), "_button") || contains(img->entity(), "_arrow");
  });

  if (collision != "")
    set<C::String>("Interface:active_menu_item", collision);
  else
    remove ("Interface:active_menu_item", true);

  if (receive("Cursor:clicked") && collision != "")
    emit ("Menu:clicked");
}

void Control::menu_touchscreen()
{
  if (receive("Cursor:clicked"))
  {
    auto cursor = get<C::Position>(CURSOR__POSITION);

    // Detect collision with clickable objets
    std::string collision = first_collision(cursor, [&](const C::Image_handle img) -> bool
    {
      return contains(img->entity(), "_button") || contains(img->entity(), "_arrow");
    });

    if (collision != "")
    {
      set<C::String>("Interface:active_menu_item", collision);
      emit ("Menu:clicked");
    }
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

} // namespace Sosage::System
