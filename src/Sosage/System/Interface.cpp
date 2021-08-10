/*
  [src/Sosage/System/Interface.cpp]
  Handles interactions with buttons, collisions, etc.

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
#include <Sosage/Component/Menu.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Interface.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/gamepad_labels.h>

#include <queue>

namespace Sosage::System
{

namespace C = Component;

Interface::Interface (Content& content)
  : Base (content)
  , m_latest_exit (-10000)
{

}

void Interface::run()
{
  update_exit();

  if (receive("Input_mode:changed"))
    update_active_objects();

  auto status = get<C::Status>(GAME__STATUS);
  if (status->value() == PAUSED)
    return;

  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (status->value() != CUTSCENE && status->value() != LOCKED)
  {
    if (mode->value() == MOUSE || mode->value() == TOUCHSCREEN)
    {
      auto cursor = get<C::Position>(CURSOR__POSITION);
      detect_collision (cursor);

      if (receive ("Cursor:clicked") && m_collision)
      {
        if (status->value() == IN_WINDOW)
          window_clicked();
        else if (status->value() == IN_CODE)
          code_clicked(cursor);
        else if (status->value() == IN_MENU)
          menu_clicked();
        else if (status->value() == DIALOG_CHOICE)
          dialog_clicked();
        else if (status->value() == ACTION_CHOICE || status->value() == INVENTORY_ACTION_CHOICE)
          action_clicked();
        else if (status->value() == OBJECT_CHOICE)
          object_clicked();
        else if (status->value() == IN_INVENTORY)
          inventory_clicked();
        else // IDLE
          idle_clicked();
      }
    }
    else // if (mode->value() == GAMEPAD)
    {
      bool active_objects_changed = false;

      if (status->value() == IDLE)
        active_objects_changed = detect_proximity();

      if (auto right = request<C::Boolean>("Switch:right"))
      {
        switch_active_object (right->value());
        remove ("Switch:right");
        active_objects_changed = true;
      }

      std::string received_key = "";
      for (const std::string& key : {"move", "take", "inventory", "look"})
        if (receive("Action:" + key))
          received_key = key;

      if (received_key != "")
      {

        if (status->value() == IN_WINDOW)
          ;
        else if (status->value() == IN_CODE)
          ;
        else if (status->value() == IN_MENU)
          ;
        else if (status->value() == DIALOG_CHOICE)
          ;
        else if (status->value() == ACTION_CHOICE || status->value() == INVENTORY_ACTION_CHOICE)
          ;
        else if (status->value() == OBJECT_CHOICE)
          ;
        else if (status->value() == IN_INVENTORY)
          ;
        else // IDLE
        {
          idle_triggered(received_key);
          active_objects_changed = true;
        }
      }

      if (active_objects_changed)
        update_active_objects();

      update_action_selector();
      update_switcher();
    }
  }
  else
    clear_action_ids();

  update_inventory();
  update_dialog_choices();
}

void Interface::init()
{
  auto pause_screen_pos
    = set<C::Absolute_position>("Pause_screen:position", Point(0, 0));
  set<C::Variable>("Window_overlay:position", pause_screen_pos);

  auto blackscreen = set<C::Image>("Blackscreen:image",
                                   Config::world_width,
                                   Config::world_height,
                                   0, 0, 0, 255);
  blackscreen->on() = false;
  blackscreen->z() = Config::overlay_depth;
  blackscreen->set_collision(UNCLICKABLE);

  set<C::Absolute_position>("Blackscreen:position", Point(0,0));

  auto inventory_origin = set<C::Absolute_position>("Inventory:origin", Point(0, Config::world_height));

  auto inventory_label = set<C::Image>("Inventory_label:image", get<C::Font>("Interface:font"),
                                       "FFFFFF", get<C::String>("Inventory:label")->value());
  inventory_label->set_collision(UNCLICKABLE);
  inventory_label->z() = Config::inventory_depth;
  inventory_label->set_scale(0.5);
  inventory_label->set_relative_origin (0.5, 0.5);

  auto inventory_label_background = set<C::Image>("Inventory_label_background:image",
                                                  Config::label_margin + inventory_label->width() / 2,
                                                  Config::label_height);
  inventory_label_background->set_collision(BOX);
  inventory_label_background->z() = Config::interface_depth;
  int label_width = inventory_label_background->width();
  int label_height = inventory_label_background->height();
  set<C::Relative_position>("Inventory_label_background:position", inventory_origin, Vector(0, -label_height));
  set<C::Relative_position>("Chamfer:position", inventory_origin, Vector(label_width, -label_height));
  get<C::Image>("Chamfer:image")->set_collision(BOX);
  set<C::Relative_position>("Inventory_label:position", inventory_origin, Vector(label_width / 2, - 0.5 * label_height));

  auto inventory_background = set<C::Image>("Inventory_background:image", Config::world_width, Config::inventory_height);
  inventory_background->z() = Config::interface_depth;
  inventory_background->set_collision(UNCLICKABLE);
  set<C::Relative_position>("Inventory_background:position", inventory_origin);

  set<C::Relative_position>("Left_arrow:position", inventory_origin, Vector(Config::inventory_margin, Config::inventory_height / 2));
  set<C::Relative_position>("Right_arrow:position", inventory_origin, Vector(Config::world_width - Config::inventory_margin, Config::inventory_height / 2));

  init_menus();
}

void Interface::window_clicked()
{
  auto window = get<C::Image>("Game:window");
  window->on() = false;
  get<C::Status>(GAME__STATUS)->pop();
}

void Interface::code_clicked (C::Position_handle cursor)
{
  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");
  if (m_collision != window)
  {
    window->on() = false;
    code->reset();
    get<C::Status>(GAME__STATUS)->pop();
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
      = std::atoi(std::string(m_collision->id().begin() +
                              std::ptrdiff_t(std::string("Dialog_choice_").size()),
                              m_collision->id().begin() +
                              std::ptrdiff_t(m_collision->id().find(':'))).c_str());

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

  get<C::Status>(GAME__STATUS)->pop();
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
  auto status = get<C::Status>(GAME__STATUS);
  status->pop();

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
    get<C::Status>(GAME__STATUS)->push(OBJECT_CHOICE);
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
      std::cerr << action_id << std::endl;
      set_action (action_id, "Default_inventory");
      m_target = "";
      action_clicked();
    }
    else
    {
      get<C::Status>(GAME__STATUS)->pop();
      m_target = "";
    }
  }
  else if (m_collision->entity().find("Inventory_arrow") == 0)
    arrow_clicked();
  else
  {
    get<C::Status>(GAME__STATUS)->pop();
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
      std::cerr << action_id << std::endl;
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
      get<C::Status>(GAME__STATUS)->pop();
      get<C::Status>(GAME__STATUS)->push(INVENTORY_ACTION_CHOICE);
      emit ("Click:play_sound");
    }
  }
  else if (m_collision->entity().find("Inventory_arrow") == 0)
    arrow_clicked();
}

void Interface::idle_clicked()
{
  std::string id = m_collision->entity();

  // Click on an object
  if (request<C::String>(id + ":name"))
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
      get<C::Status>(GAME__STATUS)->push(ACTION_CHOICE);
      emit ("Click:play_sound");
    }
    else // Source exist, search for action ID/source
    {
      std::string action_id = id + "_inventory_" + m_source;
      std::cerr << action_id << std::endl;
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

void Interface::idle_triggered (const std::string& action)
{
  if (m_active_object == "")
  {
    if (action == "inventory")
    {
      get<C::Status>(GAME__STATUS)->push (IN_INVENTORY);
      m_active_object = get<C::Inventory>("Game:inventory")->get(0);
    }
    return;
  }

  if (action == "inventory")
  {
    m_target = m_active_object;
    get<C::Status>(GAME__STATUS)->push (OBJECT_CHOICE);
    m_active_object = get<C::Inventory>("Game:inventory")->get(0);
  }
  set_action (m_active_object + "_" + action, "Default_" + action);
}

void Interface::set_action (const std::string& id, const std::string& default_id)
{
  if (auto action = request<C::Action>(id + ":action"))
    set<C::Variable>("Character:action", action);
  else
    set<C::Variable>("Character:action", get<C::Action>(default_id + ":action"));
}

void Interface::update_pause_screen()
{
  auto interface_font = get<C::Font> ("Interface:font");

  auto pause_screen_img
    = C::make_handle<C::Image>
    ("Pause_screen:image",
     Config::world_width,
     Config::world_height,
     0, 0, 0, 192);
  pause_screen_img->z() += 10;

  // Create pause screen
  auto status
    = get<C::Status>(GAME__STATUS);

  auto pause_screen
    = set<C::Conditional>("Pause_screen:conditional",
                                            C::make_value_condition<Sosage::Status>(status, PAUSED),
                                            pause_screen_img);

  auto pause_text_img
    = C::make_handle<C::Image>("Pause_text:image", interface_font, "FFFFFF", "PAUSE");
  pause_text_img->z() += 10;
  pause_text_img->set_relative_origin(0.5, 0.5);

  auto window_overlay_img
    = set<C::Image>("Window_overlay:image",
                                      Config::world_width,
                                      Config::world_height,
                                      0, 0, 0, 128);
  window_overlay_img->z() = Config::interface_depth;
  window_overlay_img->on() = false;

  auto pause_text
    = set<C::Conditional>("Pause_text:conditional",
                                            C::make_value_condition<Sosage::Status>(status, PAUSED),
                                            pause_text_img);

  set<C::Absolute_position>("Pause_text:position", Point(Config::world_width / 2,
                                                Config::world_height / 2));
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

  auto status = get<C::Status>(GAME__STATUS);

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

  if (status->value() == IDLE
      && cursor->value().y() > Config::world_height - Config::inventory_active_zone)
  {
    status->push (IN_INVENTORY);
    return;
  }

  if (status->value() == IN_INVENTORY
      && cursor->value().y() < Config::world_height - Config::inventory_height
      && m_collision->id() != "Chamfer:image"
      && m_collision->id() != "Inventory_label_background:image")
  {
    status->pop();
    return;
  }

  if (status->value() == IDLE && m_collision &&
      (m_collision->id() == "Chamfer:image" ||
       m_collision->id() == "Inventory_label_background:image"))
  {
    status->push(IN_INVENTORY);
    return;
  }


  if (previous_collision && (previous_collision != m_collision))
  {
    const std::string& id = previous_collision->entity();
    if (status->value() == ACTION_CHOICE || status->value() == INVENTORY_ACTION_CHOICE)
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
    else
    {
      clear_action_ids();
    }
  }

  if (m_collision)
  {
    const std::string& id = m_collision->entity();
    if (status->value() == ACTION_CHOICE || status->value() == INVENTORY_ACTION_CHOICE)
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
    else
    {
      bool display_label = status->value() == IDLE;
      if (status->value() == OBJECT_CHOICE || status->value() == IN_INVENTORY)
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

          update_label(false, id + "_label", name->value(), true, false, cursor->value(), UNCLICKABLE);
          if (force_right || get<C::Position>(id + "_label_right_circle:position")->value().x() >
              Config::world_width - Config::label_height)
              update_label(false, id + "_label", name->value(), false, true, cursor->value(), UNCLICKABLE);
        }
      }
      else
        clear_action_ids();
    }
  }
}

bool Interface::detect_proximity()
{
  const std::string& id = get<C::String>("Player:name")->value();
  auto position = get<C::Position>(id + "_body:position");

  // Find objects with labels close to player
  std::unordered_set<std::string> close_objects;
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
          close_objects.insert (label->entity());

        // Object in hysteresis range
        else if (m_close_objects.find(label->entity()) != m_close_objects.end())
          close_objects.insert (label->entity());
      }

  if (close_objects == m_close_objects)
    return false;

  if (m_active_object == "" && !close_objects.empty())
    m_active_object = *close_objects.begin();

  // If active object is not in reach anymore, find closest to activate
  else if (close_objects.find(m_active_object) == close_objects.end())
  {
    std::string chosen = "";
    double dx_min = std::numeric_limits<double>::max();
    auto active_pos = get<C::Position>(m_active_object + ":label");
    for (const std::string& id : close_objects)
    {
      auto pos = get<C::Position>(id + ":position");
      double dx = std::abs (active_pos->value().x() - pos->value().x());
      if (dx < dx_min)
      {
        chosen = id;
        dx_min = dx;
      }
    }
    m_active_object = chosen;
  }

  m_close_objects.swap(close_objects);

  return true;
}

void Interface::switch_active_object (const bool& right)
{
  if (m_close_objects.size() < 2)
    return;

  std::vector<std::string> close_objects;
  close_objects.reserve (m_close_objects.size());
  std::copy (m_close_objects.begin(), m_close_objects.end(),
             std::back_inserter (close_objects));

  // Sort close objects by X coordinate to switch to the immediate left or right
  std::sort (close_objects.begin(), close_objects.end(),
             [&](const std::string& a, const std::string& b) -> bool
             {
               auto pos_a = get<C::Position>(a + ":position");
               auto pos_b = get<C::Position>(b + ":position");
               return pos_a->value().x() < pos_b->value().x();
             });

  for (std::size_t i = 0; i < close_objects.size(); ++ i)
    if (close_objects[i] == m_active_object)
    {
      m_active_object = (right ? close_objects[(i + 1) % close_objects.size()]
                         : close_objects[(i + close_objects.size() - 1) % close_objects.size()]);
      return;
    }

  dbg_check(false, "Switch active object failed");
}

void Interface::clear_action_ids(bool clear_highlights)
{
  if (clear_highlights)
    for (C::Group_handle group : m_labels)
    {
      const std::string& id = group->id();
      std::size_t pos = id.find("_label:group");
      if (pos != std::string::npos)
      {
        std::string img_id (id.begin(), id.begin() + pos);
        if (auto img = request<C::Image>(img_id + ":image"))
          img->set_highlight(0);
      }
    }

  for (C::Group_handle group : m_labels)
  {
    std::cerr << "Remove group " << group->id() << std::endl;
    group->apply<C::Base> ([&](auto h) { remove (h->id()); });
    remove (group->id());
  }
  m_labels.clear();
}

void Interface::update_label (bool is_button, const std::string& id, std::string name,
                              bool open_left, bool open_right, const Point& position,
                              const Collision_type& collision, double scale)
{
  auto pos = set<C::Absolute_position>(id + ":global_position", position);
  auto group = request<C::Group>(id + ":group");
  C::Image_handle label, left, right, back;

  unsigned char alpha = (is_button ? 255 : 100);
  int depth = (is_button ? Config::action_button_depth : Config::label_depth);

  if (group)
  {
    label = request<C::Image>(id + ":image");
    left = request<C::Image>(id + "_left_circle:image");
    right = request<C::Image>(id + "_right_circle:image");
    back = request<C::Image>(id + "_back:image");
  }
  else
  {
    group = set<C::Group>(id + ":group");
    m_labels.push_back (group);

    if (name != "")
    {
      name[0] = toupper(name[0]);
      label = set<C::Image>(id + ":image", get<C::Font>("Interface:font"), "FFFFFF", name);
      group->add(label);

      label->set_relative_origin(0.5, 0.5);
      label->set_scale(scale * 0.5);
      label->z() = depth;
      label->set_collision(collision);
      label->set_alpha(scale * 255);
    }

    left = set<C::Image>(id + "_left_circle:image", get<C::Image>("Left_circle:image"));
    group->add(left);

    left->on() = true;
    left->set_relative_origin(1, 0.5);
    left->set_scale(scale);
    left->set_alpha(alpha);
    left->z() = depth - 1;
    left->set_collision(collision);

    right = set<C::Image>(id + "_right_circle:image", get<C::Image>("Right_circle:image"));
    group->add(right);

    right->on() = true;
    right->set_relative_origin(0, 0.5);
    right->set_scale(scale);
    right->set_alpha(alpha);
    right->z() = depth - 1;
    right->set_collision(collision);

    if (label)
    {
      int margin = Config::label_margin;
      if (m_source != "")
        margin *= 2;
      else if (open_left && open_right)
        margin *= 3;

      int width = margin + label->width() / 2;
      if (is_button || name.size() == 1)
        width = (name.size() - 1) * Config::label_margin;

      if (width != 0)
      {
        back = set<C::Image>(id + "_back:image", width, Config::label_height);
        group->add(back);

        back->set_relative_origin(0.5, 0.5);
        back->set_scale(scale);
        back->set_alpha(alpha);
        back->z() = depth - 1;
        back->set_collision(collision);
      }
    }
  }

  if (left)
    left->on() = !open_left;
  if (right)
    right->on() = !open_right;


  int half_width_minus = (back ? round(scale * back->width()) / 2 : 0);
  int half_width_plus = (back ? round(scale * back->width()) - half_width_minus : 0);

  // Dirty as fuck but I can't find a cleaner way to avoid gaps between circles and back in some configurations…
  if (back && scale != 1.0 && round(scale * back->width()) % 2 == 1)
  {
   half_width_plus --;
   half_width_minus ++;
  }

  if(open_left == open_right) // symmetric label
  {
    set<C::Relative_position>(id + ":position", pos);
    set<C::Relative_position>(id + "_back:position", pos);
  }
  else if (open_left)
  {
    set<C::Relative_position>(id + ":position", pos, Vector(scale * Config::label_diff + half_width_plus, 0));
    set<C::Relative_position>(id + "_back:position", pos, Vector(half_width_plus, 0));
  }
  else if (open_right)
  {
    set<C::Relative_position>(id + ":position", pos, Vector(-scale * Config::label_diff - half_width_minus, 0));
    set<C::Relative_position>(id + "_back:position", pos, Vector(-half_width_minus, 0));
  }

  if (left)
    set<C::Relative_position>(id + "_left_circle:position",
                     get<C::Position>(id + "_back:position"), Vector(-half_width_minus, 0));
  if (right)
    set<C::Relative_position>(id + "_right_circle:position",
                               get<C::Position>(id + "_back:position"), Vector(half_width_plus, 0));
}

void Interface::generate_action (const std::string& id, const std::string& action,
                                 const Button_orientation& orientation, const std::string& button,
                                 Point position)
{
  auto label = request<C::String>(id + "_" + action + ":label");
  if (action == "cancel")
    label = get<C::String>("Cancel:text");
  if (!label)
      label = get<C::String>("Default_" + action + ":label");
  if (id == "Default" && action == "inventory")
    label = get<C::String>("Inventory:label");

  bool open_left = false, open_right = false;
  if (position == Point())
    position = get<C::Position>(CURSOR__POSITION)->value();
  Point label_position;
  Point button_position;

  // Default cross orientation
  if (orientation == UP)
  {
    label_position = position + Vector(0, -80);
    button_position = position + Vector(0, -40);
  }
  else if (orientation == DOWN)
  {
    label_position = position + Vector(0, 80);
    button_position = position + Vector(0, 40);
  }
  else if (orientation == RIGHT_BUTTON)
  {
    label_position = position + Vector(40, 0);
    button_position = position + Vector(40, 0);
    open_left = true;
  }
  else if (orientation == LEFT_BUTTON)
  {
    label_position = position + Vector(-40, 0);
    button_position = position + Vector(-40, 0);
    open_right = true;
  }

  // Side orientations
  else if (orientation == UPPER)
  {
    label_position = position + Vector(0, -100);
    button_position = position + Vector(0, -60);
  }
  else if (orientation == DOWNER)
  {
    label_position = position + Vector(0, 100);
    button_position = position + Vector(0, 60);
  }
  else if (orientation == UP_RIGHT)
  {
    label_position = position + Vector(50, -28.25);
    button_position = position + Vector(50, -28.25);
    open_left = true;
  }
  else if (orientation == UP_LEFT)
  {
    label_position = position + Vector(-50, -28.25);
    button_position = position + Vector(-50, -28.25);
    open_right = true;
  }
  else if (orientation == DOWN_RIGHT)
  {
    label_position = position + Vector(50, 28.25);
    button_position = position + Vector(50, 28.25);
    open_left = true;
  }
  else if (orientation == DOWN_LEFT)
  {
    label_position = position + Vector(-50, 28.25);
    button_position = position + Vector(-50, 28.25);
    open_right = true;
  }

  if (id != "")
  {
    update_label (false, id + "_" + action + "_label", label->value(), open_left, open_right,
                  label_position, BOX);

    // UPPER and DOWNER configs might need to be moved to be on screen
    if (orientation == UPPER || orientation == DOWNER)
    {
      int diff = 0;
      auto lpos = get<C::Position>(id + "_" + action + "_label_left_circle:position");
      if (lpos->value().x() < Config::label_height)
        diff = Config::label_height - lpos->value().x();
      else
      {
        auto rpos = get<C::Position>(id + "_" + action + "_label_right_circle:position");
        if (rpos->value().x() > Config::world_width - Config::label_height)
          diff = lpos->value().x() - (Config::world_width - Config::label_height);
      }
      if (diff != 0)
      {
        auto pos = get<C::Position>(id + "_" + action + "_label:global_position");
        pos->set (Point (pos->value().x() + diff, pos->value().y()));
      }
    }
  }

  if (id == "")
  {
    update_label (true, "Default_" + action + "_button", button, false, false, button_position, BOX);
    get<C::Image>("Default_" + action + "_button:image")->on() = false;
    get<C::Image>("Default_" + action + "_button_left_circle:image")->set_alpha(128);
    get<C::Image>("Default_" + action + "_button_right_circle:image")->set_alpha(128);
  }
  else
    update_label (true, id + "_" + action + "_button", button, false, false, button_position, BOX);
}

void Interface::update_active_objects()
{
  clear_action_ids(true);

  if (auto group = request<C::Group>("Action_selector:group"))
  {
    group->apply<C::Image>([&](auto img) { remove(img->id()); });
    group->apply<C::Group>([&](auto g) { remove(g->id()); });
    remove (group->id());
  }

  for (const std::string& id : m_close_objects)
  {
    bool is_active = (m_active_object == id);
    auto name = get<C::String>(id + ":name");
    get<C::Image>(id + ":image")->set_highlight(is_active ? 192 : 64);

    auto pos = get<C::Position>(id + ":label");

    double scale = (is_active ? 1.0 : 0.75);
    update_label(false, id + "_label", name->value(), false, false, pos->value(), UNCLICKABLE, scale);
  }

}

void Interface:: update_inventory ()
{
  Status status = get<C::Status>(GAME__STATUS)->value();
  Input_mode mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  auto inventory_origin = get<C::Absolute_position>("Inventory:origin");
  if (status == IN_INVENTORY || status == OBJECT_CHOICE || status == INVENTORY_ACTION_CHOICE)
    inventory_origin->set (Point (0, Config::world_height - Config::inventory_height));
  else if ((mode == MOUSE || mode == TOUCHSCREEN) && status == IDLE)
    inventory_origin->set (Point (0, Config::world_height));
  else
    inventory_origin->set(Point (0, 2 * Config::world_height)); // hidden way at the bottom

  auto inventory = get<C::Inventory>("Game:inventory");

  constexpr int inventory_margin = 100;
  constexpr int inventory_width = Config::world_width - inventory_margin * 2;

  std::size_t position = inventory->position();
  for (std::size_t i = 0; i < inventory->size(); ++ i)
  {
    auto img = get<C::Image>(inventory->get(i) + ":image");
    double factor = 1.;

    if (img == m_collision)
    {
      img->set_highlight(0);
      factor = 1.1;
    }

    if (position <= i && i < position + Config::displayed_inventory_size)
    {
      std::size_t pos = i - position;
      double relative_pos = (1 + pos) / double(Config::displayed_inventory_size + 1);
      img->on() = true;
      img->set_scale (0.8);

      int x = inventory_margin + int(relative_pos * inventory_width);
      int y = Config::inventory_height / 2;

      set<C::Relative_position>(inventory->get(i) + ":position", inventory_origin, Vector(x,y));
    }
    else
      img->on() = false;
  }


  get<C::Image>("Left_arrow:image")->on() = (inventory->position() > 0);
  get<C::Image>("Right_arrow:image")->on() = (inventory->size() - inventory->position()
                                              > Config::displayed_inventory_size);
}

void Interface::update_dialog_choices()
{
  if (get<C::Status>(GAME__STATUS)->value() != DIALOG_CHOICE)
    return;

  const std::vector<std::string>& choices
      = get<C::Vector<std::string> >("Dialog:choices")->value();

  // Generate images if not done yet
  if (!request<C::Image>("Dialog_choice_background:image"))
  {
    auto interface_font = get<C::Font> ("Dialog:font");
    const std::string& player = get<C::String>("Player:name")->value();

    int bottom = Config::world_height;
    int y = bottom - 10;

    for (int c = int(choices.size()) - 1; c >= 0; -- c)
    {
      std::string entity = "Dialog_choice_" + std::to_string(c);
      auto img_off
        = set<C::Image>(entity + "_off:image", interface_font, "FFFFFF",
                        choices[std::size_t(c)]);
      img_off->z() = Config::dialog_depth;
      img_off->set_scale(0.75);
      img_off->set_relative_origin(0., 1.);

      auto img_on
        = set<C::Image>(entity + "_on:image", interface_font,
                        get<C::String>(player + ":color")->value(),
                        choices[std::size_t(c)]);
      img_on->z() = Config::dialog_depth;
      img_on->set_scale(0.75);
      img_on->set_relative_origin(0., 1.);
      y -= img_off->height() * 0.75;

    }

    auto background = set<C::Image> ("Dialog_choice_background:image",
                                     Config::world_width, bottom - y + 20, 0, 0, 0, 192);
    background->set_relative_origin(0., 1.);
    set<C::Absolute_position>("Dialog_choice_background:position", Point(0,bottom));
  }

  int bottom = Config::world_height;
  int y = bottom - 10;

  for (int c = int(choices.size()) - 1; c >= 0; -- c)
  {
    std::string entity = "Dialog_choice_" + std::to_string(c);
    auto img_off = get<C::Image>(entity + "_off:image");
    Point p (10, y);
    set<C::Absolute_position>(entity + "_off:position", p);
    set<C::Absolute_position>(entity + "_on:position", p);
    y -= img_off->height() * 0.75;
  }

  auto cursor = get<C::Position>(CURSOR__POSITION);

  for (int c = int(choices.size()) - 1; c >= 0; -- c)
  {
    std::string entity = "Dialog_choice_" + std::to_string(c);
    auto img_off = get<C::Image>(entity + "_off:image");
    auto img_on = get<C::Image>(entity + "_on:image");
    const Point& p = get<C::Position>(entity + "_off:position")->value();

    Point screen_position = p - img_off->core().scaling * Vector(img_off->origin());
    int xmin = screen_position.X();
    int ymin = screen_position.Y();
    int xmax = xmin + int(img_off->core().scaling * (img_off->xmax() - img_off->xmin()));
    int ymax = ymin + int(img_off->core().scaling * (img_off->ymax() - img_off->ymin()));

    bool on = (xmin <= cursor->value().x() && cursor->value().x() <= xmax &&
               ymin <= cursor->value().y() && cursor->value().y() <= ymax);
    img_off->on() = !on;
    img_on->on() = on;
  }
}

void Interface::update_action_selector()
{
  auto group = request<C::Group>("Action_selector:group");
  if (!group)
  {
    group = set<C::Group>("Action_selector:group");

    std::size_t nb_labels = m_labels.size();
    Point origin (Config::world_width - 240, Config::world_height - 130);

    const Gamepad_type& gamepad = get<C::Simple<Gamepad_type>>("Gamepad:type")->value();

    std::string take_id = "";
    std::string look_id = "";
    std::string move_id = "";
    std::string inventory_id = "Default";
    std::string take_action = "take";
    std::string look_action = "look";
    std::string move_action = "move";
    std::string inventory_action = "inventory";
    if (m_active_object != "")
    {
      take_id = m_active_object;
      look_id = m_active_object;
      move_id = m_active_object;
      inventory_id = m_active_object;
    }
    if (get<C::Status>(GAME__STATUS)->value() == IN_INVENTORY)
    {
      origin = origin + Vector(0, -Config::inventory_height);
      take_action = "combine";
      move_action = "use";
      inventory_action = "cancel";
    }

    generate_action (take_id, take_action, LEFT_BUTTON, gamepad_label(gamepad, WEST), origin);
    generate_action (look_id, look_action, RIGHT_BUTTON, gamepad_label(gamepad, EAST), origin);
    generate_action (move_id, move_action, UP, gamepad_label(gamepad, NORTH), origin);
    generate_action (inventory_id, inventory_action, DOWN, gamepad_label(gamepad, SOUTH), origin);

    for (std::size_t i = nb_labels; i < m_labels.size(); ++ i)
      group->add(m_labels[i]);
    m_labels.resize (nb_labels);
  }
}

void Interface::update_switcher()
{
  auto group = request<C::Group>("Switcher:group");
  if (!group)
  {
    bool keyboard = (get<C::Simple<Gamepad_type>>(GAMEPAD__TYPE)->value() == KEYBOARD);

    if (keyboard)
      update_label (true, "Switcher_left", "Tab", false, false, Point(0,0), UNCLICKABLE);
    else
      update_label (true, "Switcher_left", "L", false, false, Point(0,0), UNCLICKABLE);
    m_labels.pop_back();

    auto left_pos = get<C::Absolute_position>("Switcher_left:global_position");
    left_pos->set (Point (Config::label_height - get<C::Position>("Switcher_left_left_circle:position")->value().x(),
                          Config::world_height - Config::label_height));


    update_label (false, "Switcher_label", get<C::String>("Switch_target:text")->value(),
                  true, !keyboard, Point(0,0), UNCLICKABLE);
    m_labels.pop_back();

    // Correct position of half-open label in keyboard mode (a bit hacky but meh…)
    if (keyboard)
    {
      get<C::Relative_position>("Switcher_label:position")->set(Vector(Config::label_margin,0));
      get<C::Relative_position>("Switcher_label_back:position")->set(Vector(0,0));
    }

    auto img = get<C::Image>("Switcher_label_back:image");
    auto pos = get<C::Absolute_position>("Switcher_label:global_position");
    pos->set (Point (get<C::Position>("Switcher_left_right_circle:position")->value().x() + img->width() / 2,
                     left_pos->value().y()));

    if (!keyboard)
    {
      update_label (true, "Switcher_right", "R", false, false, Point(0,0), UNCLICKABLE);
      m_labels.pop_back();
      auto right_pos = get<C::Absolute_position>("Switcher_right:global_position");
      right_pos->set (Point (pos->value().x() + img->width() / 2, left_pos->value().y()));
    }

    group = set<C::Group>("Switcher:group");
    group->add (get<C::Group>("Switcher_left:group"));
    group->add (get<C::Group>("Switcher_label:group"));
    if (auto g = request<C::Group>("Switcher_right:group"))
      group->add(g);
  }

  group->apply<C::Image> ([&](auto img) { img->on() = (m_close_objects.size() >= 2); });
}


} // namespace Sosage::System
