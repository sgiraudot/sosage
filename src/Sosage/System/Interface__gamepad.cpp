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

void Interface::idle_triggered (const std::string& action)
{
  if (m_active_object == "")
  {
    if (action == "inventory")
    {
      status()->push (IN_INVENTORY);
      m_active_object = get<C::Inventory>("Game:inventory")->get(0);
    }
    return;
  }

  if (action == "inventory")
  {
    m_target = m_active_object;
    status()->push (OBJECT_CHOICE);
    m_active_object = get<C::Inventory>("Game:inventory")->get(0);
  }
  set_action (m_active_object + "_" + action, "Default_" + action);
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
    if (status()->value() == IN_INVENTORY)
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
