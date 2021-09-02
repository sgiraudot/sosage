/*
  [src/Sosage/System/Interface__mouse.cpp]
  Handles mouse (and touch) interactions.

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
#include <Sosage/Component/Group.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Interface.h>

namespace Sosage::System
{

namespace C = Component;


void Interface::window_clicked()
{
  auto window = get<C::Image>("Game:window");
  window->on() = false;
  status()->pop();
}

void Interface::code_clicked (C::Position_handle cursor)
{
  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");
  if (m_collision != window)
  {
    window->on() = false;
    code->reset();
    status()->pop();
  }
  else
  {
    auto position
      = get<C::Position>(window->entity() + ":position");

    Point p = cursor->value() - Vector(position->value()) + Vector (0.5  * window->width(),
                                                                    0.5 * window->height());
    if (code->click(p.X(), p.Y()))
      emit ("code:button_clicked");
  }
}

void Interface::dialog_clicked ()
{
  if (m_collision->entity().find("Dialog_choice_") == std::string::npos)
    return;

  const std::vector<std::string>& choices
      = get<C::Vector<std::string> >("Dialog:choices")->value();

  int choice
      = to_int(std::string(m_collision->id().begin() +
                           std::ptrdiff_t(std::string("Dialog_choice_").size()),
                           m_collision->id().begin() +
                           std::ptrdiff_t(m_collision->id().find(':'))));

  set<C::Int>("Dialog:choice", choice);

  // Clean up
  for (int c = int(choices.size()) - 1; c >= 0; -- c)
  {
    std::string entity = "Dialog_choice_" + std::to_string(c);
    remove(entity + "_off:image");
    remove(entity + "_off:position");
    remove(entity + "_on:image");
    remove(entity + "_on:position");
  }
  remove("Dialog_choice_background:image");
  remove("Dialog_choice_background:position");
  remove("Game:current_dialog");

  status()->pop();
  emit ("Click:play_sound");
}

void Interface::arrow_clicked()
{
  if (m_collision->entity().find("_0") != std::string::npos ||
      m_collision->entity().find("_1") != std::string::npos)
    get<C::Inventory>("Game:inventory")->prev();
  else
    get<C::Inventory>("Game:inventory")->next();
}

void Interface::action_clicked()
{
  status()->pop();

  const std::string& id = m_collision->entity();

  std::size_t pos = id.find("_button_");
  if (pos == std::string::npos)
    pos = id.find("_label");

  clear_action_ids();

  if (pos == std::string::npos)
    return;

  std::string object_id (id.begin(), id.begin() + pos);

  pos = object_id.find_last_of('_');
  check(pos != std::string::npos, "Ill-formed action entity " + object_id);
  std::string target (object_id.begin(), object_id.begin() + pos);
  std::string action (object_id.begin() + pos + 1, object_id.end());

  if (action == "inventory")
  {
    m_target = target;
    status()->push(OBJECT_CHOICE);
  }
  else if (action == "combine")
  {
    m_source = target;
    get<C::String>("Cursor:state")->set("selected");

    auto cursor_img = C::make_handle<C::Image>("Selected_object:image", get<C::Image>(target + ":image"));
    cursor_img->set_scale(0.28);
    cursor_img->set_collision(UNCLICKABLE);
    cursor_img->z() = Config::cursor_depth+1;

    auto cursor_cond = set<C::String_conditional>("Selected_object:image", get<C::String>("Cursor:state"));
    cursor_cond->add("selected", cursor_img);
    set<C::Variable>("Selected_object:position", get<C::Position>(CURSOR__POSITION));
  }
  else
    set_action (object_id, "Default_" + action);
  emit ("Click:play_sound");
}

void Interface::object_clicked()
{
  // Click happen while OBJECT_CHOIE
  std::string id = m_collision->entity();

  // If object is clicked
  if (request<C::String>(id + ":name"))
  {
    // and object is indeed inventory object!
    auto state = request<C::String>(m_collision->entity() + ":state");
    if (state && state->value() == "inventory")
    {
      std::string action_id = m_target + "_inventory_" + id;
      set_action (action_id, "Default_inventory");
      m_target = "";
      action_clicked();
    }
    else
    {
      status()->pop();
      m_target = "";
    }
  }
  else if (m_collision->entity().find("Inventory_arrow") == 0)
    arrow_clicked();
  else
  {
    status()->pop();
    m_target = "";
  }
}

void Interface::inventory_clicked()
{
  // Click happen while IN_INVENTORY
  std::string id = m_collision->entity();

  // If object is clicked
  if (request<C::String>(id + ":name"))
  {
    if (m_source != "") // Source exists, search for action ID/source
    {
      std::string action_id = id + "_inventory_" + m_source;
      set_action (action_id, "Default_inventory");
      m_source = "";
      get<C::String>("Cursor:state")->set("default");
    }
    else // No target, no source, create actions for inventory object
    {
      clear_action_ids();
      const Point& object_pos = get<C::Position>(id + ":position")->value();
      Point position (object_pos.x(),
                      get<C::Position>("Inventory:origin")->value().y() - 0.75 * Config::label_height);

      generate_action (id, "combine", LEFT_BUTTON, "", position);
      double diff = get<C::Position>(id + "_combine_label_left_circle:position")->value().x()
                    - (get<C::Position>("Chamfer:position")->value().x() + Config::label_height);
      if (diff < 0)
      {
        position = Point (position.x() - diff, position.y());
        generate_action (id, "combine", LEFT_BUTTON, "", position);
      }
      generate_action (id, "use", UP, "", position);
      generate_action (id, "look", RIGHT_BUTTON, "", position);
      status()->pop();
      status()->push(INVENTORY_ACTION_CHOICE);
      emit ("Click:play_sound");
    }
  }
  else if (m_collision->entity().find("Inventory_arrow") == 0)
    arrow_clicked();
}

void Interface::idle_clicked()
{
  std::string id = m_collision->entity();

  bool object_is_active = true;
  if (get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value() == TOUCHSCREEN)
  {
    if (std::find(m_close_objects.begin(), m_close_objects.end(), id) == m_close_objects.end())
      object_is_active = false;
    else
      m_close_objects.clear();
  }

  // Click on an object
  if (request<C::String>(id + ":name") && object_is_active)
  {
    // Object is a path to other room
    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      set_action(id + "_goto", "Default_goto");
      emit ("Click:play_sound");
    }
    // No source exists
    else if (m_source == "")
    {
      clear_action_ids();
      generate_action (id, "take", LEFT_BUTTON);
      if (get<C::Position>(id + "_take_label_left_circle:position")->value().x() < Config::label_height)
      {
        generate_action (id, "move", UPPER);
        generate_action (id, "take", UP_RIGHT);
        generate_action (id, "look", DOWN_RIGHT);
        generate_action (id, "inventory", DOWNER);
      }
      else
      {
        generate_action (id, "look", RIGHT_BUTTON);
        if (get<C::Position>(id + "_take_label_right_circle:position")->value().x()
            > Config::world_width - Config::label_height)
        {
          generate_action (id, "move", UPPER);
          generate_action (id, "take", UP_LEFT);
          generate_action (id, "look", DOWN_LEFT);
          generate_action (id, "inventory", DOWNER);

        }
        else
        {
          generate_action (id, "move", UP);
          generate_action (id, "inventory", DOWN);
        }
      }
      status()->push(ACTION_CHOICE);
      emit ("Click:play_sound");
    }
    else // Source exist, search for action ID/source
    {
      std::string action_id = id + "_inventory_" + m_source;
      if (auto action = request<C::Action>(action_id + ":action"))
        set<C::Variable>("Character:action", action);
      else
        set<C::Variable>("Character:action", get<C::Action>("Default_inventory:action"));
      m_source = "";
      get<C::String>("Cursor:state")->set("default");
    }
  }
  else // Click anywhere else
  {
    if (m_source == "") // No source
      emit ("Cursor:clicked"); // Logic handles this click
    else // If source, cancel it
    {
      m_source = "";
      get<C::String>("Cursor:state")->set("default");
    }
  }
}


void Interface::detect_collision (C::Position_handle cursor)
{
  // Deactive previous collisions
  if (m_collision)
  {
    if (auto name = request<C::String>(m_collision->entity() + ":name"))
    {
      if (auto img = request<C::Image>(m_collision->entity() + ":image"))
        img->set_highlight(0);
      if (m_source == "") // Keep source cursor if it exists
        get<C::String>("Cursor:state")->set("default");
    }
  }

  bool touchmode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value() == TOUCHSCREEN;

  auto previous_collision = m_collision;
  m_collision = C::Image_handle();
  double xcamera = get<C::Double>(CAMERA__POSITION)->value();

  const std::string& player = get<C::String>("Player:name")->value();

  for (const auto& e : m_content)
    if (auto img = C::cast<C::Image>(e))
    {
      if (!img->on() ||
          img->collision() == UNCLICKABLE ||
          img->character_entity() == player ||
          img->id().find("debug") == 0 ||
          img->id().find("Interface_") == 0)
        continue;

      auto position = get<C::Position>(img->entity() + ":position");
      Point p = position->value();

      if (auto absol = C::cast<C::Absolute_position>(position))
        if (!absol->absolute())
          p = p + Vector (-xcamera, 0);

      Point screen_position = p - img->core().scaling * Vector(img->origin());
      int xmin = screen_position.X();
      int ymin = screen_position.Y();
      int xmax = xmin + int(img->core().scaling * (img->xmax() - img->xmin()));
      int ymax = ymin + int(img->core().scaling * (img->ymax() - img->ymin()));

      if (cursor->value().x() < xmin ||
          cursor->value().x() >= xmax ||
          cursor->value().y() < ymin ||
          cursor->value().y() >= ymax)
        continue;

      if (img->collision() == PIXEL_PERFECT)
      {
        int x_in_image = cursor->value().X() - xmin;
        int y_in_image = cursor->value().Y() - ymin;

        if (!img->is_target_inside (x_in_image, y_in_image))
          continue;
      }

      // Now, collision happened

      if (m_collision)
      {
        // Keep image closest to screen
        if (img->z() > m_collision->z())
          m_collision = img;
      }
      else
        m_collision = img;

    }

  if (status()->value() == IDLE
      && cursor->value().y() > Config::world_height - Config::inventory_active_zone)
  {
    status()->push (IN_INVENTORY);
    return;
  }

  if (status()->value() == IN_INVENTORY
      && cursor->value().y() < Config::world_height - Config::inventory_height
      && m_collision->id() != "Chamfer:image"
      && m_collision->id() != "Inventory_label_background:image")
  {
    status()->pop();
    return;
  }

  if (status()->value() == IDLE && m_collision &&
      (m_collision->id() == "Chamfer:image" ||
       m_collision->id() == "Inventory_label_background:image"))
  {
    status()->push(IN_INVENTORY);
    return;
  }

  if (touchmode && status()->value() == IDLE)
  {
    std::string id = m_collision->entity();
    std::size_t pos = id.find("_label");
    if (pos != std::string::npos)
    {
      id.resize(pos);
      m_collision = get<C::Image>(id + ":image");
    }
  }


  if (previous_collision && (previous_collision != m_collision))
  {
    const std::string& id = previous_collision->entity();
    if (status()->value() == ACTION_CHOICE || status()->value() == INVENTORY_ACTION_CHOICE)
    {
      std::size_t pos = id.find("_button_");
      if (pos == std::string::npos)
        pos = id.find("_label");
      if (pos != std::string::npos)
      {
        std::string object_id (id.begin(), id.begin() + pos);
        get<C::Image>(object_id + "_button_left_circle:image")->set_highlight(0);
        get<C::Image>(object_id + "_button_right_circle:image")->set_highlight(0);
      }
    }
    else if (!touchmode)
    {
      clear_action_ids();
    }
  }

  if (m_collision)
  {
    const std::string& id = m_collision->entity();
    if (status()->value() == ACTION_CHOICE || status()->value() == INVENTORY_ACTION_CHOICE)
    {
      std::size_t pos = id.find("_button_");
      if (pos == std::string::npos)
        pos = id.find("_label");
      if (pos != std::string::npos)
      {
        std::string object_id (id.begin(), id.begin() + pos);
        get<C::Image>(object_id + "_button_left_circle:image")->set_highlight(255);
        get<C::Image>(object_id + "_button_right_circle:image")->set_highlight(255);
      }
    }
    else if (status()->value() == IN_CODE && !touchmode)
    {
      auto code = get<C::Code>("Game:code");
      auto window = get<C::Image>("Game:window");
      auto position
        = get<C::Position>(window->entity() + ":position");

      Point p = cursor->value() - Vector(position->value()) + Vector (0.5  * window->width(),
                                                                      0.5 * window->height());

      if (code->hover(p.X(), p.Y()))
        generate_code_hover();
      else
        remove("Code_hover:image", true);
    }
    else
    {
      bool display_label = !touchmode && status()->value() == IDLE;
      if (status()->value() == OBJECT_CHOICE || status()->value() == IN_INVENTORY)
        if (auto state = request<C::String>(id + ":state"))
          if (state->value() == "inventory")
            display_label = true;

      if (display_label)
      {
        if (auto name = request<C::String>(id + ":name"))
        {
          get<C::Image>(m_collision->entity() + ":image")->set_highlight(128);
          bool force_right = false;
          if (m_source == "")
          {
            if (auto right = request<C::Boolean>(id + "_goto:right"))
            {
              if (right->value())
              {
                get<C::String>("Cursor:state")->set("goto_right");
                force_right = true;
              }
              else
                get<C::String>("Cursor:state")->set("goto_left");
            }
            else
              get<C::String>("Cursor:state")->set("object");
          }

          update_label(false, id + "_label", locale(name->value()), true, false, cursor->value(), UNCLICKABLE);
          if (force_right || get<C::Position>(id + "_label_right_circle:position")->value().x() >
              Config::world_width - Config::label_height)
              update_label(false, id + "_label", locale(name->value()), false, true, cursor->value(), UNCLICKABLE);
        }
      }
      else if (!touchmode)
        clear_action_ids();
    }
  }
}



} // namespace Sosage::System
