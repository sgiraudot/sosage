/*
  [src/Sosage/System/Interface__gamepad.cpp]
  Handles gamepad (and keyboard) interactions.

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
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/gamepad_labels.h>

#include <queue>

namespace Sosage::System
{

namespace C = Component;

void Interface::window_triggered(const std::string& action)
{
  if (action == "look" || action == "inventory")
  {
    auto window = get<C::Image>("Game:window");
    window->on() = false;
    status()->pop();
  }
}

void Interface::code_triggered(const std::string& action)
{
  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");
  if (action == "inventory")
  {
    window->on() = false;
    code->reset();
    remove("Code_hover:image");
    status()->pop();
  }
  else if (action == "look")
  {
    if (code->click())
      emit ("code:button_clicked");
  }
}

void Interface::menu_triggered(const std::string& action)
{
  const std::string& menu = get<C::String>("Game:current_menu")->value();
  bool settings = (menu == "Settings");

  if (action == "up")
    switch_active_object(false);
  else if (action == "down")
    switch_active_object(true);
  else if (action == "left")
  {
    if (settings)
    {
      if (m_active_object.find("_button") == std::string::npos)
        menu_clicked(m_active_object + "_left_arrow");
    }
    else
      switch_active_object(false);
  }
  else if (action == "right")
  {
    if (settings)
    {
      if (m_active_object.find("_button") == std::string::npos)
        menu_clicked(m_active_object + "_right_arrow");
    }
    else
      switch_active_object(true);
  }
  else if (action == "look")
  {
    if (settings && m_active_object.find("_button") == std::string::npos)
      menu_clicked(m_active_object + "_right_arrow");
    else
      menu_clicked();
  }
  else if (action == "inventory")
  {
    emit("Game:escape");
    update_exit();
  }
}

void Interface::dialog_triggered (const std::string& action)
{
  if (action != "look")
    return;

  std::size_t pos = m_active_object.find_last_of('_');
  std::string current_str (m_active_object.begin() + pos + 1, m_active_object.end());
  int choice = to_int(current_str);

  set<C::Int>("Dialog:choice", choice);

  const std::vector<std::string>& choices
      = get<C::Vector<std::string> >("Dialog:choices")->value();

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

  m_active_object = "";
  status()->pop();
  emit ("Click:play_sound");
}

void Interface::inventory_triggered(const std::string& action)
{
  if (status()->value() == OBJECT_CHOICE)
  {
    if (action == "look")
    {
      if (m_target != "")
      {
        std::string action_id = m_target + "_inventory_" + m_active_object;
        set_action (action_id, "Default_inventory");
        m_target = "";
      }
      else // if (m_source != "")
      {
        std::string action_id = m_active_object + "_inventory_" + m_source;
        set_action (action_id, "Default_inventory");
        m_source = "";
      }
      m_active_object = "";
      status()->pop();
      emit("Click:play_sound");

    }
    else if (action == "inventory")
    {
      status()->pop();
      m_source = "";
      m_target = "";
      m_active_object = "";
      emit("Click:play_sound");
    }
  }
  else // if (status()->value() == INVENTORY)
  {
    if (action == "look")
    {
      std::string action_id = m_active_object + "_look";
      set_action (action_id, "Default_look");
      m_active_object = "";
      status()->pop();
      emit("Click:play_sound");
    }
    else if (action == "move") // use
    {
      std::string action_id = m_active_object + "_use";
      set_action (action_id, "Default_use");
      m_active_object = "";
      status()->pop();
      emit("Click:play_sound");
    }
    else if (action == "take") // combine
    {
      if (get<C::Inventory>("Game:inventory")->size() > 1)
      {
        m_source = m_active_object;
        status()->pop();
        status()->push(IN_INVENTORY);

        auto inventory = get<C::Inventory>("Game:inventory");
        for (std::size_t i = 0; i < inventory->size(); ++ i)
          if (inventory->get(i) == m_active_object)
          {
            if (i == inventory->size() - 1)
              m_active_object = inventory->get(i-1);
            else
              m_active_object = inventory->get(i+1);
            break;
          }
      }
      emit("Click:play_sound");
    }
    else // if (action == "inventory")
    {
      status()->pop();
      m_active_object = "";
      m_source = "";
      emit("Click:play_sound");
    }
  }
}

void Interface::idle_triggered (const std::string& action)
{
  if (m_active_object == "")
  {
    if (action == "inventory")
    {
      status()->push (IN_INVENTORY);
      m_active_object = get<C::Inventory>("Game:inventory")->get(0);
      clear_active_objects();
      emit("Click:play_sound");
    }
    return;
  }

  if (auto right = request<C::Boolean>(m_active_object + "_goto:right"))
  {
    if (action == "look")
    {
      set_action (m_active_object + "_goto", "Default_goto");
      m_active_object = "";
      clear_active_objects();
      emit("Click:play_sound");
    }
    else if (action == "inventory")
    {
      status()->push (IN_INVENTORY);
      m_active_object = get<C::Inventory>("Game:inventory")->get(0);
      clear_active_objects();
      emit("Click:play_sound");
    }
    return;
  }

  if (action == "inventory")
  {
    m_target = m_active_object;
    status()->push (OBJECT_CHOICE);
    m_active_object = get<C::Inventory>("Game:inventory")->get(0);
    clear_active_objects();
    emit("Click:play_sound");
    return;
  }
  set_action (m_active_object + "_" + action, "Default_" + action);
  clear_active_objects();
  emit("Click:play_sound");
}


bool Interface::detect_proximity()
{
  if (status()->value() == IDLE)
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
    else if (m_active_object != "" && close_objects.find(m_active_object) == close_objects.end())
    {
      std::string chosen = close_objects.empty() ? "" : *close_objects.begin();
      auto active_pos = request<C::Position>(m_active_object + ":label");
      if (active_pos)
      {
        double dx_min = std::numeric_limits<double>::max();
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
      }
      m_active_object = chosen;
    }

    clear_active_objects();
    m_close_objects.swap(close_objects);
    return true;
  }
  else if (status()->value() == IN_WINDOW || status()->value() == IN_CODE)
  {
    if (!m_close_objects.empty())
    {
      clear_active_objects();
      return true;
    }
    return false;
  }
  else if (status()->value() == DIALOG_CHOICE)
  {
    if (m_active_object == "")
      m_active_object = "Dialog_choice_0";
    if (!m_close_objects.empty())
    {
      clear_active_objects();
      m_active_object = "Dialog_choice_0";
      return true;
    }
    return false;
  }
  return false;
}

void Interface::switch_active_object (const bool& right)
{
  if (status()->value() == IDLE)
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
      auto pos_a = get<C::Position>(a + "_label:global_position");
      auto pos_b = get<C::Position>(b + "_label:global_position");
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
  else if (status()->value() == IN_INVENTORY || status()->value() == OBJECT_CHOICE)
  {
    auto inventory = get<C::Inventory>("Game:inventory");

    for (std::size_t i = 0; i < inventory->size(); ++ i)
      if (inventory->get(i) == m_active_object)
      {
        int diff = 0;
        if (right && i < inventory->size() - 1)
          diff = 1;
        else if (!right && i > 0)
          diff = -1;

        if (inventory->get(i+diff) == m_source)
        {
          diff = 0;
          if (right && i < inventory->size() - 2)
            diff = 2;
          else if (!right && i > 1)
            diff = -2;
        }
        m_active_object = inventory->get(i+diff);
        break;
      }
  }
  else if (status()->value() == DIALOG_CHOICE)
  {
    const std::vector<std::string>& choices
        = get<C::Vector<std::string> >("Dialog:choices")->value();

    std::size_t pos = m_active_object.find_last_of('_');
    std::string current_str (m_active_object.begin() + pos + 1, m_active_object.end());
    std::size_t current = to_int(current_str);
    if (right)
    {
      if (current < choices.size() - 1)
        m_active_object = "Dialog_choice_" + std::to_string(current+1);
      else
        m_active_object = "Dialog_choice_0";
    }
    else
    {
      if (current > 0)
        m_active_object = "Dialog_choice_" + std::to_string(current-1);
      else
        m_active_object = "Dialog_choice_" + std::to_string(choices.size() - 1);
    }
  }
  else if (status()->value() == IN_MENU)
  {
    const std::string& id = get<C::String>("Game:current_menu")->value();
    auto menu = get<C::Menu>(id + ":menu");
    bool settings = (id == "Settings");

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
        if (!nodes.empty() && nodes.back() == m_active_object)
          nb_current = nodes.size() - 1;
      }
      for (std::size_t i = 0; i < current.nb_children(); ++ i)
        todo.push (current[i]);
    }

    check (nb_current != std::size_t(-1), "Node " + m_active_object + " not found in menu");
    if (right)
    {
      if (nb_current < nodes.size() - 1)
        m_active_object = nodes[nb_current + 1];
      else
        m_active_object = nodes.front();
    }
    else
    {
      if (nb_current > 0)
        m_active_object = nodes[nb_current - 1];
      else
        m_active_object = nodes.back();
    }

  }
}

void Interface::clear_active_objects()
{
  for (const std::string& id : m_close_objects)
    if (auto img = request<C::Image>(id + ":image"))
      img->set_highlight(0);
  m_close_objects.clear();
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

  bool touchmode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value() == TOUCHSCREEN;

  double xcamera = get<C::Double>(CAMERA__POSITION)->value();

  for (const std::string& id : m_close_objects)
  {
    bool is_active = touchmode || (m_active_object == id);
    auto name = get<C::String>(id + ":name");
    get<C::Image>(id + ":image")->set_highlight(is_active ? 192 : 64);

    auto pos = get<C::Position>(id + ":label")->value() + Vector(-xcamera, 0);

    double scale = (is_active ? 1.0 : 0.75);
    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      bool r = right->value();
      update_label(false, id + "_label", locale(name->value()), !r, r, pos,
                   touchmode ? BOX : UNCLICKABLE, scale, true);
    }
    else
      update_label(false, id + "_label", locale(name->value()), false, false, pos,
                   touchmode ? BOX : UNCLICKABLE, scale);
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
      if (auto right = request<C::Boolean>(m_active_object + "_goto:right"))
      {
        take_id = "";
        move_id = "";
        inventory_id = "Default";
        look_action = "goto";
      }
    }
    if (status()->value() == IN_INVENTORY)
    {
      origin = origin + Vector(0, -Config::inventory_height);
      take_action = "combine";
      move_action = "use";
      inventory_action = "Cancel";
      if (get<C::Inventory>("Game:inventory")->size() == 1)
        take_id = "";
    }
    else if (status()->value() == OBJECT_CHOICE)
    {
      take_id = "";
      move_id = "";
      origin = origin + Vector(0, -Config::inventory_height);
      look_action = "Ok";
      inventory_action = "Cancel";
    }
    else if (status()->value() == IN_CODE || status()->value() == IN_WINDOW)
    {
      look_id = "code";
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Cancel";
    }
    else if (status()->value() == IN_MENU)
    {
      look_id = "menu";
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Continue";
    }

    generate_action (take_id, take_action, LEFT_BUTTON, gamepad_label(gamepad, WEST), origin);
    generate_action (look_id, look_action, RIGHT_BUTTON, gamepad_label(gamepad, EAST), origin);
    generate_action (move_id, move_action, UP, gamepad_label(gamepad, NORTH), origin);
    generate_action (inventory_id, inventory_action, DOWN, gamepad_label(gamepad, SOUTH), origin);

    for (std::size_t i = nb_labels; i < m_labels.size(); ++ i)
      group->add(m_labels[i]);
    m_labels.resize (nb_labels);
  }

  if (status()->value() == LOCKED || status()->value() == DIALOG_CHOICE)
    group->apply<C::Image> ([&](auto img) { img->on() = false; });
  else
    group->apply<C::Image> ([&](auto img) { img->on() = true; });
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


    update_label (false, "Switcher_label", locale_get("Switch_target:text"),
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
