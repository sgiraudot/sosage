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
#include <Sosage/Component/GUI_animation.h>
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
#if 0
  , m_latest_status(IDLE)
  , m_latest_exit (-10000)
  , m_stick_on (false)
#endif
{

}

void Interface::run()
{
  update_active_objects();
  update_action_selector();
  update_object_switcher();
  update_inventory();
  update_code_hover();
  update_menu();
  update_cursor();
}

void Interface::init()
{
  // Init black screen
  auto blackscreen = set<C::Image>("Blackscreen:image",
                                   Config::world_width,
                                   Config::world_height,
                                   0, 0, 0, 255);
  blackscreen->on() = false;
  blackscreen->z() = Config::overlay_depth;
  blackscreen->set_collision(UNCLICKABLE);

  set<C::Absolute_position>("Blackscreen:position", Point(0,0));

  // Init inventory
  auto inventory_origin = set<C::Absolute_position>
                          ("Inventory:origin",
                           Point(0, Config::world_height + Config::inventory_active_zone));

  auto inventory_label = set<C::Image>("Inventory_label:image", get<C::Font>("Interface:font"),
                                       "FFFFFF", locale_get("Inventory:label"));
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

  // Init object switchers
  create_label (true, "Keyboard_switcher_left", "Tab", false, false, UNCLICKABLE);
  auto kb_left_pos = set<C::Absolute_position>("Keyboard_switcher_left:global_position", Point(0,0));
  update_label ("Keyboard_switcher_left", false, false, kb_left_pos);
  kb_left_pos->set (Point (Config::label_height - get<C::Position>("Keyboard_switcher_left_left_circle:position")->value().x(),
                        Config::world_height - Config::label_height));

  create_label (true, "Gamepad_switcher_left", "L", false, false, UNCLICKABLE);
  auto left_pos = set<C::Absolute_position>("Gamepad_switcher_left:global_position", Point(0,0));
  update_label ("Gamepad_switcher_left", false, false, left_pos);
  left_pos->set (Point (Config::label_height - get<C::Position>("Gamepad_switcher_left_left_circle:position")->value().x(),
                        Config::world_height - Config::label_height));

  create_label (false, "Keyboard_switcher_label", locale_get("Switch_target:text"), true, false, UNCLICKABLE);
  auto kb_img = get<C::Image>("Keyboard_switcher_label_back:image");
  auto kb_pos = set<C::Absolute_position>("Keyboard_switcher_label:global_position", Point(0,0));
  update_label ("Keyboard_switcher_label", true, false, kb_pos);
  kb_pos->set (Point (get<C::Position>("Keyboard_switcher_left_right_circle:position")->value().x() + kb_img->width() / 2,
                      kb_left_pos->value().y()));
  get<C::Relative_position>("Keyboard_switcher_label:position")->set(Vector(Config::label_margin,0));
  get<C::Relative_position>("Keyboard_switcher_label_back:position")->set(Vector(0,0));

  create_label (false, "Gamepad_switcher_label", locale_get("Switch_target:text"), true, true, UNCLICKABLE);
  auto img = get<C::Image>("Gamepad_switcher_label_back:image");
  auto pos = set<C::Absolute_position>("Gamepad_switcher_label:global_position", Point(0,0));
  update_label ("Gamepad_switcher_label", true, true, pos);
  pos->set (Point (get<C::Position>("Gamepad_switcher_left_right_circle:position")->value().x() + img->width() / 2,
                      left_pos->value().y()));

  create_label (true, "Gamepad_switcher_right", "R", false, false, UNCLICKABLE);
  auto right_pos = set<C::Absolute_position>("Gamepad_switcher_right:global_position", Point(0,0));
  update_label ("Gamepad_switcher_right", false, false, right_pos);
  right_pos->set (Point (pos->value().x() + img->width() / 2, left_pos->value().y()));

  auto kb_switcher = set<C::Group>("Keyboard_switcher:group");
  kb_switcher->add (get<C::Group>("Keyboard_switcher_left:group"));
  kb_switcher->add (get<C::Group>("Keyboard_switcher_label:group"));
  kb_switcher->apply<C::Image> ([&](auto img) { img->set_alpha(0); });

  auto gamepad_switcher = set<C::Group>("Gamepad_switcher:group");
  gamepad_switcher->add (get<C::Group>("Gamepad_switcher_left:group"));
  gamepad_switcher->add (get<C::Group>("Gamepad_switcher_label:group"));
  gamepad_switcher->add (get<C::Group>("Gamepad_switcher_right:group"));
  gamepad_switcher->apply<C::Image> ([&](auto img) { img->set_alpha(0); });

  // Init gamepad action selector position
  set<C::Relative_position>("Gamepad_action_selector:position",
                            inventory_origin, Vector (Config::world_width - 240, -Config::inventory_active_zone - 130));

  set<C::Variable>("Selected_object:position", get<C::Position>(CURSOR__POSITION));

  init_menus();
}

void Interface::update_active_objects()
{
  if (auto active_objects = request<C::Vector<std::string>>("Interface:active_objects"))
  {
    if (get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value() == GAMEPAD)
    {
      auto active_object = get<C::String>("Interface:active_object");

      // No change
      if (active_objects->value() == m_active_objects
          && active_object->value() == m_active_object)
        return;

      std::string old_selected = m_active_object;
      std::string new_selected = active_object->value();

      m_active_object = active_object->value();

      std::unordered_set<std::string> new_active, old_active, all_active;
      for (const std::string& a : m_active_objects)
      {
        old_active.insert(a);
        all_active.insert(a);
      }
      for (const std::string& a : active_objects->value())
      {
        new_active.insert(a);
        all_active.insert(a);
      }

      debug("Updating labels:");
      for (const std::string& a : all_active)
      {
        // Object was active and is not anymore
        if (!contains(new_active, a))
        {
          debug(a + " is not active anymore");
          highlight_object (a, 0);
          delete_label (a + "_label");
        }
        // Object was selected and is not anymore (but still here)
        else if (a == old_selected && a != new_selected)
        {
          debug(a + " is not selected anymore");
          update_label_position(a, 0.75);
          animate_label(a + "_label", ZOOM);
          highlight_object (a, 192);
        }
        // Object was not active and is now active
        else if (!contains(old_active, a) && contains(new_active, a))
        {
          debug(a + " is now active");
          create_object_label (a);
          highlight_object (a, (a == new_selected ? 255 : 192));
        }
        // Object was already here but is now selected
        else if (a != old_selected && a == new_selected)
        {
          debug(a + " is now selected");
          update_label_position(a, 1.0);
          animate_label(a + "_label", ZOOM);
          highlight_object (a, 255);
        }
      }
    }
    else // TOUCHSCREEN
    {
      // No change
      if (active_objects->value() == m_active_objects)
        return;

      std::unordered_set<std::string> new_active, old_active, all_active;
      for (const std::string& a : m_active_objects)
      {
        old_active.insert(a);
        all_active.insert(a);
      }
      for (const std::string& a : active_objects->value())
      {
        new_active.insert(a);
        all_active.insert(a);
      }

      debug("Updating labels:");
      for (const std::string& a : all_active)
      {
        // Object was active and is not anymore
        if (!contains(new_active, a))
        {
          debug(a + " is not active anymore");
          highlight_object (a, 0);
          delete_label (a + "_label");
        }
        // Object was not active and is now active
        else if (!contains(old_active, a) && contains(new_active, a))
        {
          debug(a + " is now active");
          create_object_label (a);
          highlight_object (a, 255);
        }
      }
    }

    m_active_objects = active_objects->value();
  }
  else if (!m_active_objects.empty())
  {
    // Deactivate objects not active anymore
    for (const std::string& a : m_active_objects)
    {
      highlight_object (a, 0);
      delete_label (a + "_label");
    }
    m_active_objects.clear();
    m_active_object = "";
  }
  else if (auto active = request<C::String>("Interface:active_object"))
  {
    if (status()->is (INVENTORY_ACTION_CHOICE))
    {
      if (m_active_object != "")
      {
        highlight_object (m_active_object, 0);
        delete_label (m_active_object + "_label");
      }
    }
    // Active object didn't change, just update position
    else if (m_active_object == active->value())
      update_label_position(m_active_object); // Just update position
    // New active object
    else
    {
      // If previous object, deactivate it first
      if (m_active_object != "")
      {
        highlight_object (m_active_object, 0);
        delete_label (m_active_object + "_label");
      }

      // Then activate object
      m_active_object = active->value();
      create_object_label (m_active_object);
      highlight_object (m_active_object, 255);
    }
  }
  // If previous object, deactivate it
  else if (m_active_object != "")
  {
    highlight_object (m_active_object, 0);
    delete_label (m_active_object + "_label");
    m_active_object = "";
  }
}

void Interface::update_action_selector()
{
  const Input_mode& mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  if (mode == GAMEPAD)
  {
    if (auto target = request<C::String>("Interface:active_object"))
    {
      // Mouse action selector might be active, deactivate too
      bool garbage_mouse_selector = false;
      if (auto p = request<C::Relative_position>(m_action_selector[0] + "_button_back:position"))
        if (p->absolute_reference() != get<C::Absolute_position>("Inventory:origin"))
          garbage_mouse_selector = true;

      // Action selector not up to date
      bool uptodate = (status()->is (IDLE) && m_action_selector[0] == target->value() + "_move")
          || (status()->is (IN_INVENTORY) && m_action_selector[0] == target->value() + "_use")
          || (status()->is (OBJECT_CHOICE) && m_action_selector[1] == target->value() + "_Ok");

      if (garbage_mouse_selector || !uptodate)
      {
        if (m_action_selector[0] != "")
          reset_action_selector();
        set_action_selector (target->value());
      }
    }
    else
    {
      bool uptodate = (status()->is (IN_WINDOW, IN_CODE) && m_action_selector[1] == "code_Ok")
          || (status()->is (IN_MENU) && m_action_selector[1] == "menu_Ok")
          || m_action_selector[2] == "Default_inventory";

      // Action selector not up to date
      if (!uptodate)
      {
        if (m_action_selector[0] != "")
          reset_action_selector();
        set_action_selector ("");
      }
    }
  }
  else
  {
    if (auto target = request<C::String>("Interface:action_choice_target"))
    {
      // Gamepad action selector might be active, deactivate too
      bool garbage_gamepad_selector = false;
      if (auto p = request<C::Relative_position>(m_action_selector[0] + "_button_back:position"))
        if (p->absolute_reference() == get<C::Absolute_position>("Inventory:origin"))
          garbage_gamepad_selector = true;

      // Action selector not up to date
      bool uptodate = (status()->is (ACTION_CHOICE) && m_action_selector[0] == target->value() + "_move")
          || (status()->is (INVENTORY_ACTION_CHOICE) && m_action_selector[0] == target->value() + "_use");

      // Action selector not up to date
      if (garbage_gamepad_selector || !uptodate)
      {
        if (m_action_selector[0] != "")
          reset_action_selector();
        set_action_selector(target->value());
      }
    }
    else
      if (m_action_selector[0] != "")
        reset_action_selector();

    if (m_action_selector[0] != "")
    {
      std::string active_button = "";
      if (auto a = request<C::String>("Interface:active_button"))
      {
        active_button = a->value();
        std::size_t pos = active_button.find("_button_");
        if (pos == std::string::npos)
          pos = active_button.find("_label");
        if (pos != std::string::npos)
          active_button.resize(pos);
      }
      for (const std::string& id : m_action_selector)
      {
        if (id == "")
          continue;
        unsigned char highlight = (id == active_button ? 255 : 0);
        get<C::Image>(id + "_button_left_circle:image")->set_highlight(highlight);
        get<C::Image>(id + "_button_right_circle:image")->set_highlight(highlight);
      }
    }
  }
}

void Interface::update_object_switcher()
{
  bool keyboard_on = false;
  bool gamepad_on = false;

  const Input_mode& mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();
  if (mode == GAMEPAD)
    if (auto active_objects = request<C::Vector<std::string>>("Interface:active_objects"))
      if (active_objects->value().size() > 1)
      {
        keyboard_on = (get<C::Simple<Gamepad_type>>(GAMEPAD__TYPE)->value() == KEYBOARD);
        gamepad_on = !keyboard_on;
      }

  if (keyboard_on && get<C::Image>("Keyboard_switcher_left:image")->alpha() == 0)
    fade_action_selector ("Keyboard_switcher", true);
  else if (gamepad_on && get<C::Image>("Gamepad_switcher_left:image")->alpha() == 0)
    fade_action_selector ("Gamepad_switcher", true);
  else if (!keyboard_on && get<C::Image>("Keyboard_switcher_left:image")->alpha() == 255)
    fade_action_selector ("Keyboard_switcher", false);
  else if (!gamepad_on && get<C::Image>("Gamepad_switcher_left:image")->alpha() == 255)
    fade_action_selector ("Gamepad_switcher", false);
}

void Interface::update_inventory()
{
  Input_mode mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  auto inventory_origin = get<C::Absolute_position>("Inventory:origin");

  double target = 0;
  double as_target = 0;
  if (status()->is (IN_INVENTORY, OBJECT_CHOICE, INVENTORY_ACTION_CHOICE))
  {
    target = Config::world_height - Config::inventory_height;
    as_target = target - 80 - 2 * Config::inventory_margin;
  }
  else if ((mode == MOUSE || mode == TOUCHSCREEN) && status()->is (IDLE))
  {
    target = Config::world_height;
    as_target = target - Config::inventory_active_zone - 130;
  }
  else
  {
    target = Config::inventory_active_zone + Config::world_height; // hidden at the bottom
    as_target = target - Config::inventory_active_zone - 130;
  }

  if (target != inventory_origin->value().y() && !request<C::GUI_animation>("Inventory:animation"))
  {
    double current_time = get<C::Double>(CLOCK__TIME)->value();
    auto position = get<C::Position>("Inventory:origin");
    set<C::GUI_position_animation> ("Inventory:animation", current_time, current_time + Config::inventory_speed,
                                    position, Point(0, target));

    auto as_pos = get<C::Position>("Gamepad_action_selector:position");
    set<C::GUI_position_animation> ("Gamepad_action_selector:animation", current_time, current_time + Config::inventory_speed,
                                    as_pos, Point(Config::world_width - 240, as_target));
  }

  auto inventory = get<C::Inventory>("Game:inventory");

  constexpr int inventory_margin = 100;
  constexpr int inventory_width = Config::world_width - inventory_margin * 2;

  std::string active_object = "";
  if (auto a = request<C::String>("Interface:active_object"))
    active_object = a->value();
  std::string source_object = "";
  if (auto s = request<C::String>("Interface:source_object"))
    source_object = s->value();

  std::size_t position = inventory->position();
  for (std::size_t i = 0; i < inventory->size(); ++ i)
  {
    auto img = get<C::Image>(inventory->get(i) + ":image");
    double factor = 0.7;
    unsigned char alpha = 255;
    unsigned char highlight = 0;

    if (inventory->get(i) == active_object)
    {
      highlight = 255;
      factor = 0.9;
    }
    if (inventory->get(i) == source_object)
    {
      highlight = 0;
      alpha = 128;
      factor = 0.5;
    }

    if (position <= i && i < position + Config::displayed_inventory_size)
    {
      std::size_t pos = i - position;
      double relative_pos = (1 + pos) / double(Config::displayed_inventory_size + 1);
      img->on() = true;
      img->set_scale (factor);
      img->set_alpha (alpha);
      img->set_highlight(highlight);

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

void Interface::update_code_hover()
{
  if (!receive("Code:hover"))
    return;

  // Possible improvment: avoid creating image at each frame
  const std::string& player = get<C::String>("Player:name")->value();
  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");
  auto position
    = get<C::Position>(window->entity() + ":position");

  const std::string& color_str = get<C::String>(player + ":color")->value();
  RGB_color color = color_from_string (color_str);
  auto img = set<C::Image>("Code_hover:image", code->xmax() - code->xmin(), code->ymax() - code->ymin(),
                           color[0], color[1], color[2], 128);
  img->set_collision(UNCLICKABLE);
  img->z() = Config::inventory_depth;
  set<C::Absolute_position>
      ("Code_hover:position", Point(code->xmin(), code->ymin())
       + Vector(position->value())
       - Vector (0.5  * window->width(),
                 0.5 * window->height()));
}

void Interface::update_cursor()
{
  const Input_mode& mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  if (mode == MOUSE)
  {
    auto state = get<C::String>("Cursor:state");
    if (auto source = request<C::String>("Interface:source_object"))
    {
      if (state->value() != "selected")
      {
        state->set("selected");
        auto cursor_img = C::make_handle<C::Image>("Selected_object:image",
                                                   get<C::Image>(source->value() + ":image"));
        cursor_img->set_alpha(255);
        cursor_img->set_scale(0.28);
        cursor_img->set_collision(UNCLICKABLE);
        cursor_img->z() = Config::cursor_depth+1;

        auto cursor_cond = set<C::String_conditional>("Selected_object:image", state);
        cursor_cond->add("selected", cursor_img);
      }
    }
    else
    {
      if (state->value() == "selected")
        remove("Selected_object:image");

      if (!status()->is(INVENTORY_ACTION_CHOICE) && m_active_object != "")
      {
        if (auto right = request<C::Boolean>(m_active_object + "_goto:right"))
        {
          if (right->value())
            state->set("goto_right");
          else
            state->set("goto_left");
        }
        else
          state->set("object");
      }
      else
        state->set("default");
    }
  }
}

void Interface::create_object_label (const std::string& id)
{
  debug("Create object_label " + id);
  // Special case for inventory
  if (status()->is (IN_INVENTORY, OBJECT_CHOICE))
  {
    auto position = get<C::Position>(id + ":position");
    auto pos = set<C::Relative_position>(id + "_label:global_position", position,
                                         Vector(0, -Config::inventory_height / 2 - 2 * Config::inventory_margin));

    const std::string& name = get<C::String>(id + ":name")->value();
    create_label (false, id + "_label", locale(name), false, false, UNCLICKABLE);
    update_label(id + "_label", false, false, pos);

    double diff = get<C::Position>(id + "_label_left_circle:position")->value().x()
                  - (get<C::Position>("Chamfer:position")->value().x() + Config::label_height);
    if (diff < 0)
      position->set(Point (position->value().x() - diff, position->value().y()));

    return;
  }

  const Input_mode& mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  if (mode == MOUSE)
  {
    bool force_right = false;
    if (!request<C::String>("Interface:source_object"))
      if (auto right = request<C::Boolean>(id + "_goto:right"))
        force_right = right->value();

    const std::string& name = get<C::String>(id + ":name")->value();
    create_label (false, id + "_label", locale(name), true, false, UNCLICKABLE);

    auto cursor = get<C::Position>(CURSOR__POSITION);
    update_label(id + "_label", true, false, cursor);
    if (force_right || get<C::Position>(id + "_label_right_circle:position")->value().x() >
        Config::world_width - Config::label_height)
      update_label(id + "_label", false, true, cursor);
  }
  else
  {
    bool open_left = false;
    bool open_right = false;

    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      open_left = !right->value();
      open_right = right->value();
    }

    bool is_active = (mode == TOUCHSCREEN) || (m_active_object == id);
    double scale = (is_active ? 1.0 : 0.75);

    const std::string& name = get<C::String>(id + ":name")->value();
    create_label (false, id + "_label", locale(name), open_left, open_right,
                  (mode == TOUCHSCREEN) ? BOX : UNCLICKABLE, scale, true);

    auto pos = set<C::Relative_position>
               (id + "_label:global_position",
                get<C::Position>(CAMERA__POSITION),
                get<C::Position>(id + ":label")->value(), -1.);

    update_label(id + "_label", open_left, open_right, pos, scale, true);
  }

  animate_label (id + "_label", FADE);
}

void Interface::create_label (bool is_button, const std::string& id, std::string name,
                              bool open_left, bool open_right,
                              const Collision_type& collision, double scale, bool arrow)
{
  auto group = set<C::Group>(id + ":group");
  int depth = (is_button ? Config::action_button_depth : Config::label_depth);
  unsigned char alpha = (is_button ? 255 : 100);

  C::Image_handle label, left, right, back;
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

  if (open_left && arrow)
    left = set<C::Image>(id + "_left_circle:image", get<C::Image>("Goto_left:image"));
  else
  {
    left = set<C::Image>(id + "_left_circle:image", get<C::Image>("Left_circle:image"));
    left->set_alpha(alpha);
  }
  group->add(left);

  left->on() = true;
  left->set_relative_origin(1, 0.5);
  left->set_scale(scale);
  left->z() = depth - 1;
  left->set_collision(collision);

  if (open_right && arrow)
    right = set<C::Image>(id + "_right_circle:image", get<C::Image>("Goto_right:image"));
  else
  {
    right = set<C::Image>(id + "_right_circle:image", get<C::Image>("Right_circle:image"));
    right->set_alpha(alpha);
  }
  group->add(right);

  right->on() = true;
  right->set_relative_origin(0, 0.5);
  right->set_scale(scale);
  right->z() = depth - 1;
  right->set_collision(collision);

  if (label)
  {
    int margin = Config::label_margin;
    if (request<C::String>("Interface:source_object"))
      margin *= 2;
    else if (open_left && open_right)
      margin *= 3;

    int width = margin + label->width() / 2;
    if (is_button || name.size() == 1)
      width = (name.size() - 1) * Config::label_margin;
    if (scale != 1.0)
    {
      // Avoid artefacts with bad divisions
      int factor = round(1. / (1. - scale));
      while (width % factor != 0)
        ++ width;
    }

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

void Interface::animate_label (const std::string& id, const Animation_style& style,
                               bool button , const Point& position)
{
  if (style == NONE)
    return;

  double current_time = get<C::Double>(CLOCK__TIME)->value();

  if (!button)
  {
    if (style == ZOOM)
    {
      double from = 0.5 * 0.75, to = 0.5;
      if (get<C::Image>(id + ":image")->scale() != 0.5)
        std::swap(from, to);

      set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + ":image"), from, to, 255, 255);
    }
    else
    {
      double scale = get<C::Image>(id + "_left_circle:image")->scale();

      for (const std::string& element : { "back", "left_circle", "right_circle" })
      {
        auto img = get<C::Image>(id + "_" + element + ":image");
        set<C::GUI_image_animation>(id + "_back:animation", current_time, current_time + Config::inventory_speed,
                                    img, scale, scale, 0, img->alpha());
      }

      if (style == DEPLOY)
        set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id + ":image"), 0.05, 0.5 * scale, 0, 255);
      else if (style == FADE || style == FADE_LABEL_ONLY)
        set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id + ":image"), 0.5 * scale, 0.5 * scale, 0, 255);
    }
  }
  else
  {
    unsigned char alpha = get<C::Image>(id + "_left_circle:image")->alpha();
    if (style == DEPLOY)
    {
      set<C::GUI_position_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                     get<C::Position>(id + ":global_position"), position);
      set<C::GUI_image_animation>(id + "_left_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_left_circle:image"), 0.357, 1, alpha, alpha);
      set<C::GUI_image_animation>(id + "_right_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_right_circle:image"), 0.357, 1, alpha, alpha);
    }
    else if (style == FADE)
    {
      set<C::GUI_image_animation>(id + "_left_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_left_circle:image"), 1, 1, 0, alpha);
      set<C::GUI_image_animation>(id + "_right_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_right_circle:image"), 1, 1, 0, alpha);
    }
  }
}

void Interface::update_label (const std::string& id,
                              bool open_left, bool open_right, C::Position_handle pos,
                              double scale, bool arrow)
{
  auto label = request<C::Image>(id + ":image");
  auto left = request<C::Image>(id + "_left_circle:image");
  auto right = request<C::Image>(id + "_right_circle:image");
  auto back = request<C::Image>(id + "_back:image");

  // If animation was happening, finalize it before updating so that scale is final
  for (const std::string& section : { "", "_left_circle", "_right_circle", "_back" })
    if (auto anim = request<C::GUI_animation>(id + section + ":animation"))
    {
      debug("Finalize " + anim->id());
      anim->finalize();
      remove(anim->id());
    }

  if (label)
    label->set_scale(scale * 0.5);

  if (left)
  {
    left->on() = !open_left || arrow;
    left->set_scale(scale);
  }
  if (right)
  {
    right->on() = !open_right || arrow;
    right->set_scale(scale);
  }

  int half_width_minus = 0;
  int half_width_plus = 0;
  if (back)
  {
    int back_width = round (scale * back->width());
    half_width_minus = back_width / 2;
    half_width_plus = back_width - half_width_minus;
    back->set_scale(scale);
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

void Interface::update_label_position (const std::string& id, double scale)
{
  if (status()->is (IN_INVENTORY, OBJECT_CHOICE))
    return;
  debug("Update label position " + id);

  const Input_mode& mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  if (mode == MOUSE)
  {
    bool force_right = false;
    if (!request<C::String>("Interface:source_object"))
      if (auto right = request<C::Boolean>(id + "_goto:right"))
        force_right = right->value();

    auto cursor = get<C::Position>(CURSOR__POSITION);
    update_label(id + "_label", true, false, cursor, scale);
    if (force_right || get<C::Position>(id + "_label_right_circle:position")->value().x() >
        Config::world_width - Config::label_height)
      update_label(id + "_label", false, true, cursor, scale);
  }
  else
  {
    bool open_left = false;
    bool open_right = false;
    bool arrow = false;

    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      open_left = !right->value();
      open_right = right->value();
      arrow = true;
    }

    auto pos = set<C::Relative_position>
               (id + "_label:global_position",
                get<C::Position>(CAMERA__POSITION),
                get<C::Position>(id + ":label")->value(), -1.);

    update_label(id + "_label", open_left, open_right, pos, scale, arrow);
  }
}

void Interface::delete_label (const std::string& id)
{
  auto group = request<C::Group>(id + ":group");
  if (!group)
    return;
  debug("Delete label " + id);

  double current_time = get<C::Double>(CLOCK__TIME)->value();
  group->apply<C::Image>([&](auto img_old)
  {
    // To avoid later referencing a fading-out image, copy it with another ID and animate the copy
    auto img = set<C::Image>(img_old->entity() + "_old:image", img_old);
    set<C::Variable>(img_old->entity() + "_old:position", get<C::Position>(img_old->entity() + ":position"));
    remove(img_old->id());
    set<C::GUI_image_animation>(img->entity() + ":animation", current_time, current_time + Config::inventory_speed,
                                img, img->scale(), img->scale(), img->alpha(), 0, true);
  });
  remove(group->id());
}

void Interface::fade_action_selector (const std::string& id, bool fade_in)
{
  // Exit if animation already happening
  if (request<C::GUI_image_animation>(id + "_left:animation"))
    return;
  double current_time = get<C::Double>(CLOCK__TIME)->value();
  auto group = get<C::Group>(id + ":group");
  group->apply<C::Image>([&](auto img)
  {
    unsigned char alpha_off = 0;
    unsigned char alpha_on = 255;
    if (contains(img->id(), "_label") && !contains(img->id(), "_label:"))
      alpha_on = 100;

    set<C::GUI_image_animation>(img->entity() + ":animation", current_time, current_time + Config::inventory_speed,
                                img, img->scale(), img->scale(),
                                (fade_in ? alpha_off : alpha_off),
                                (fade_in ? alpha_on : alpha_off),
                                false);
  });
}

void Interface::highlight_object (const std::string& id, unsigned char highlight)
{
  debug("Highlight " + id + " by " + to_string(int(highlight)));
  // Image might have been destroyed here
  if (auto img = request<C::Image>(id + ":image"))
    img->set_highlight (highlight);
}

void Interface::set_action_selector (const std::string& id)
{
  debug("Set action selector to " + id);
  const Input_mode& mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();
  if (mode == GAMEPAD)
  {
    std::string take_id = id;
    std::string look_id = id;
    std::string move_id = id;
    std::string inventory_id = (id == "" ? "Default" : id);
    std::string take_action = "take";
    std::string look_action = "look";
    std::string move_action = "move";
    std::string inventory_action = "inventory";
    if (auto right = request<C::Boolean>(id + "_goto:right"))
    {
      take_id = "";
      move_id = "";
      inventory_id = "Default";
      look_action = "goto";
    }

    if (status()->is (IN_INVENTORY))
    {
      take_action = "combine";
      move_action = "use";
      inventory_action = "Cancel";
      if (get<C::Inventory>("Game:inventory")->size() == 1)
        take_id = "";
    }
    else if (status()->is (OBJECT_CHOICE))
    {
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Cancel";
    }
    else if (status()->is (IN_CODE, IN_WINDOW))
    {
      look_id = "code";
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Cancel";
    }
    else if (status()->is (IN_MENU))
    {
      look_id = "menu";
      take_id = "";
      move_id = "";
      look_action = "Ok";
      inventory_action = "Continue";
    }

    const Gamepad_type& gamepad = get<C::Simple<Gamepad_type>>("Gamepad:type")->value();
    auto pos = get<C::Position>("Gamepad_action_selector:position");
    generate_action (take_id, take_action, LEFT_BUTTON, gamepad_label(gamepad, WEST), pos, FADE_LABEL_ONLY);
    generate_action (look_id, look_action, RIGHT_BUTTON, gamepad_label(gamepad, EAST), pos, FADE_LABEL_ONLY);
    generate_action (move_id, move_action, UP, gamepad_label(gamepad, NORTH), pos, FADE_LABEL_ONLY);
    generate_action (inventory_id, inventory_action, DOWN, gamepad_label(gamepad, SOUTH), pos, FADE_LABEL_ONLY);
  }
  else
  {
    if (status()->is (INVENTORY_ACTION_CHOICE))
    {
      Point object_pos = get<C::Position>(id + ":position")->value();
      auto position = set<C::Absolute_position>
                      ("Action_selector:position",
                       Point (object_pos.x(),
                              get<C::Position>("Inventory:origin")->value().y() - 0.75 * Config::label_height));

      generate_action (id, "combine", LEFT_BUTTON, "", position, DEPLOY);
      double diff = get<C::Position>(id + "_combine_label_left_circle:position")->value().x()
                    - (get<C::Position>("Chamfer:position")->value().x() + Config::label_height);
      if (diff < 0)
      {
        position->set(Point (position->value().x() - diff, position->value().y()));
        generate_action (id, "combine", LEFT_BUTTON, "", position, DEPLOY);
      }

      generate_action (id, "use", UP, "", position, DEPLOY);
      generate_action (id, "look", RIGHT_BUTTON, "", position, DEPLOY);
    }
    else
    {
      auto position = set<C::Absolute_position>("Action_selector:position",
                                                get<C::Position>(CURSOR__POSITION)->value());


      generate_action (id, "take", LEFT_BUTTON, "", position, DEPLOY);
      if (get<C::Position>(id + "_take_label_left_circle:position")->value().x() < Config::label_height)
      {
        generate_action (id, "move", UPPER, "", position, DEPLOY);
        generate_action (id, "take", UP_RIGHT, "", position, DEPLOY);
        generate_action (id, "look", DOWN_RIGHT, "", position, DEPLOY);
        generate_action (id, "inventory", DOWNER, "", position, DEPLOY);
      }
      else
      {
        generate_action (id, "look", RIGHT_BUTTON, "", position, DEPLOY);
        if (get<C::Position>(id + "_take_label_right_circle:position")->value().x()
            > Config::world_width - Config::label_height)
        {
          generate_action (id, "move", UPPER, "", position, DEPLOY);
          generate_action (id, "take", UP_LEFT, "", position, DEPLOY);
          generate_action (id, "look", DOWN_LEFT, "", position, DEPLOY);
          generate_action (id, "inventory", DOWNER, "", position, DEPLOY);

        }
        else
        {
          generate_action (id, "move", UP, "", position, DEPLOY);
          generate_action (id, "inventory", DOWN, "", position, DEPLOY);
        }
      }
    }
  }
}

void Interface::reset_action_selector()
{
  debug ("Reset action selector");
  for (std::string& id : m_action_selector)
  {
    delete_label (id + "_label");
    delete_label (id + "_button");
    id = "";
  }
}

void Interface::generate_action (const std::string& id, const std::string& action,
                                 const Button_orientation& orientation, const std::string& button,
                                 C::Position_handle position, const Animation_style& style)
{
  auto label = request<C::String>(id + "_" + action + ":label");
  if (isupper(action[0]))
    label = get<C::String>(action + ":text");
  if (!label)
    label = get<C::String>("Default_" + action + ":label");
  if (id == "Default" && action == "inventory")
    label = get<C::String>("Inventory:label");

  bool open_left = false, open_right = false;
  Vector label_position;
  Vector button_position;
  Vector start_position;

  // Default cross orientation
  std::size_t selector_idx = 0;
  if (orientation == UP)
  {
    selector_idx = 0;
    label_position = Vector(0, -80);
    button_position = Vector(0, -40);
    start_position = Vector(0, -14);
  }
  else if (orientation == RIGHT_BUTTON)
  {
    selector_idx = 1;
    label_position = Vector(40, 0);
    button_position = Vector(40, 0);
    start_position = Vector(14, 0);
    open_left = true;
  }
  else if (orientation == DOWN)
  {
    selector_idx = 2;
    label_position = Vector(0, 80);
    button_position = Vector(0, 40);
    start_position = Vector(0, 14);
  }
  else if (orientation == LEFT_BUTTON)
  {
    selector_idx = 3;
    label_position = Vector(-40, 0);
    button_position = Vector(-40, 0);
    start_position = Vector(-14, 0);
    open_right = true;
  }

  // Side orientations
  else if (orientation == UPPER)
  {
    selector_idx = 0;
    label_position = Vector(0, -100);
    button_position = Vector(0, -60);
    start_position = Vector(0, -14);
  }
  else if (orientation == DOWN_RIGHT)
  {
    selector_idx = 1;
    label_position = Vector(50, 28.25);
    button_position = Vector(50, 28.25);
    start_position = Vector(14, 0);
    open_left = true;
  }
  else if (orientation == DOWN_LEFT)
  {
    selector_idx = 1;
    label_position = Vector(-50, 28.25);
    button_position = Vector(-50, 28.25);
    start_position = Vector(14, 0);
    open_right = true;
  }
  else if (orientation == DOWNER)
  {
    selector_idx = 2;
    label_position = Vector(0, 100);
    button_position = Vector(0, 60);
    start_position = Vector(0, 14);
  }
  else if (orientation == UP_RIGHT)
  {
    selector_idx = 3;
    label_position = Vector(50, -28.25);
    button_position = Vector(50, -28.25);
    start_position = Vector(-14, 0);
    open_left = true;
  }
  else if (orientation == UP_LEFT)
  {
    selector_idx = 3;
    label_position = Vector(-50, -28.25);
    button_position = Vector(-50, -28.25);
    start_position = Vector(-14, 0);
    open_right = true;
  }
  m_action_selector[selector_idx] = id + "_" + action;

  if (style != DEPLOY)
    start_position = button_position;

  if (id != "")
  {
    std::string label_id = id + "_" + action + "_label";
    create_label (false, label_id, locale(label->value()), open_left, open_right, BOX);
    update_label (label_id, open_left, open_right,
                  set<C::Relative_position>(label_id + ":global_position", position, label_position));

    // UPPER and DOWNER configs might need to be moved to be on screen
    if (orientation == UPPER || orientation == DOWNER)
    {
      int diff = 0;
      auto lpos = get<C::Position>(label_id + "_left_circle:position");
      if (lpos->value().x() < Config::label_height)
        diff = Config::label_height - lpos->value().x();
      else
      {
        auto rpos = get<C::Position>(label_id + "_right_circle:position");
        if (rpos->value().x() > Config::world_width - Config::label_height)
          diff = lpos->value().x() - (Config::world_width - Config::label_height);
      }
      if (diff != 0)
      {
        auto pos = get<C::Position>(label_id + ":global_position");
        pos->set (Point (pos->value().x() + diff, pos->value().y()));
      }
    }

    animate_label (label_id, style);
  }

  std::string button_id = "";
  if (id == "")
  {
    m_action_selector[selector_idx] = "Default_" + action;
    button_id = "Default_" + action + "_button";
    create_label (true, button_id, button, false, false, BOX);
    update_label (button_id, false, false,
                  set<C::Relative_position>(button_id + ":global_position", position, start_position));
    if (auto img = request<C::Image>("Default_" + action + "_button:image"))
      img->on() = false;
    get<C::Image>("Default_" + action + "_button_left_circle:image")->set_alpha(128);
    get<C::Image>("Default_" + action + "_button_right_circle:image")->set_alpha(128);
  }
  else
  {
    button_id = id + "_" + action + "_button";
    create_label (true, button_id, button, false, false, BOX);
    update_label (button_id, false, false,
                  set<C::Relative_position>(button_id + ":global_position", position, start_position));
  }

  animate_label (button_id, style, true, position->value() + button_position);
}



#if 0
void Interface::run_old()
{
  update_exit();

  if (receive("Input_mode:changed"))
    update_active_objects();

  if (status()->value() == PAUSED)
    return;

  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (status()->value() != CUTSCENE && status()->value() != LOCKED)
  {
    if (mode->value() == MOUSE)
    {
      auto cursor = get<C::Position>(CURSOR__POSITION);
      detect_collision (cursor);

      if (receive ("Cursor:clicked") && m_collision)
      {
        if (status()->value() == IN_WINDOW)
          window_clicked();
        else if (status()->value() == IN_CODE)
          code_clicked(cursor);
        else if (status()->value() == IN_MENU)
          menu_clicked();
        else if (status()->value() == DIALOG_CHOICE)
          dialog_clicked();
        else if (status()->value() == ACTION_CHOICE || status()->value() == INVENTORY_ACTION_CHOICE)
          action_clicked();
        else if (status()->value() == OBJECT_CHOICE)
          object_clicked();
        else if (status()->value() == IN_INVENTORY)
          inventory_clicked();
        else // IDLE
          idle_clicked();
      }
    }
    else if (mode->value() == TOUCHSCREEN)
    {
      auto cursor = get<C::Position>(CURSOR__POSITION);
      bool active_objects_changed = false;
      if (status()->value() != m_latest_status)
      {
        clear_active_objects();
        active_objects_changed = true;
      }
      if (!active_objects_changed && status()->value() == IDLE)
        active_objects_changed = detect_proximity();

      if (receive ("Cursor:clicked"))
      {
        detect_collision (cursor);
        if (status()->value() == IN_WINDOW)
          window_clicked();
        else if (status()->value() == IN_CODE)
          code_clicked(cursor);
        else if (status()->value() == IN_MENU)
          menu_clicked();
        else if (status()->value() == DIALOG_CHOICE)
          dialog_clicked();
        else if (status()->value() == ACTION_CHOICE || status()->value() == INVENTORY_ACTION_CHOICE)
          action_clicked();
        else if (status()->value() == OBJECT_CHOICE)
          object_clicked();
        else if (status()->value() == IN_INVENTORY)
          inventory_clicked();
        else // IDLE
          idle_clicked();
      }

      if (active_objects_changed)
        update_active_objects();
    }
    else // if (mode->value() == GAMEPAD)
    {
      bool active_objects_changed = false;
      if (status()->value() != m_latest_status)
      {
        clear_active_objects();
        active_objects_changed = true;
      }
      if (!active_objects_changed && status()->value() == IDLE)
        active_objects_changed = detect_proximity();

      if (status()->value() == IN_CODE)
      {
        if (!request<C::Image>("Code_hover:image"))
        {
          get<C::Code>("Game:code")->hover();
          generate_code_hover();
        }
      }

      if (auto right = request<C::Boolean>("Switch:right"))
      {
        if (status()->value() == IN_MENU)
          menu_triggered (right ? "right" : "left");
        else
          switch_active_object (right->value());
        remove ("Switch:right");
        active_objects_changed = true;
      }

      if (status()->value() != IDLE)
      {
        if (receive("Stick:moved"))
        {
          if (m_stick_on)
          {
            if (get<C::Simple<Vector>>(STICK__DIRECTION)->value() == Vector(0,0))
              m_stick_on = false;
          }
          else
          {
            m_stick_on = true;

            if (status()->value() == IN_INVENTORY || status()->value() == OBJECT_CHOICE)
            {
              if (get<C::Simple<Vector>>(STICK__DIRECTION)->value().x() < 0)
              {
                switch_active_object (false);
                active_objects_changed = true;
              }
              else if (get<C::Simple<Vector>>(STICK__DIRECTION)->value().x() > 0)
              {
                switch_active_object (true);
                active_objects_changed = true;
              }
            }
            else if (status()->value() == IN_CODE)
            {
              const Vector& direction = get<C::Simple<Vector>>(STICK__DIRECTION)->value();
              get<C::Code>("Game:code")->move(direction.x(), direction.y());
              generate_code_hover();
            }
            else if (status()->value() == DIALOG_CHOICE)
            {
              if (get<C::Simple<Vector>>(STICK__DIRECTION)->value().y() < 0)
              {
                switch_active_object (false);
                active_objects_changed = true;
              }
              else if (get<C::Simple<Vector>>(STICK__DIRECTION)->value().y() > 0)
              {
                switch_active_object (true);
                active_objects_changed = true;
              }
            }
            else if (status()->value() == IN_MENU)
            {
              double x = get<C::Simple<Vector>>(STICK__DIRECTION)->value().x();
              double y = get<C::Simple<Vector>>(STICK__DIRECTION)->value().y();
              if (std::abs(x) > std::abs(y))
              {
                if (x < 0)
                  menu_triggered("left");
                else
                  menu_triggered("right");
              }
              else
              {
                if (y < 0)
                  menu_triggered("up");
                else
                  menu_triggered("down");
              }
            }
          }
        }
      }

      std::string received_key = "";
      for (const std::string& key : {"move", "take", "inventory", "look"})
        if (receive("Action:" + key))
          received_key = key;

      if (received_key != "")
      {
        if (status()->value() == IN_WINDOW)
          window_triggered(received_key);
        else if (status()->value() == IN_CODE)
          code_triggered(received_key);
        else if (status()->value() == IN_MENU)
          menu_triggered(received_key);
        else if (status()->value() == DIALOG_CHOICE)
          dialog_triggered(received_key);
        else if (status()->value() == OBJECT_CHOICE || status()->value() == IN_INVENTORY)
        {
          inventory_triggered(received_key);
          active_objects_changed = true;
        }
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
  update_menu();

  m_latest_status = status()->value();
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
  auto pause_screen
    = set<C::Conditional>("Pause_screen:conditional",
                                            C::make_value_condition<Sosage::Status>(status(), PAUSED),
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
                                            C::make_value_condition<Sosage::Status>(status(), PAUSED),
                                            pause_text_img);

  set<C::Absolute_position>("Pause_text:position", Point(Config::world_width / 2,
                                                Config::world_height / 2));
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

  double current_time = get<C::Double>(CLOCK__TIME)->value();
  for (C::Group_handle group : m_labels)
  {
    group->apply<C::Image> ([&](auto img)
    {
      set<C::GUI_image_animation>(img->entity() + ":animation", current_time, current_time + Config::inventory_speed,
                                  img, img->scale(), img->scale(), img->alpha(), 0, true);

      //remove (h->id());
    });
    remove (group->id());
  }
  m_labels.clear();
}

bool Interface::update_label (bool is_button, const std::string& id, std::string name,
                              bool open_left, bool open_right, C::Position_handle pos,
                              const Collision_type& collision, double scale, bool arrow)
{
  auto group = request<C::Group>(id + ":group");
  C::Image_handle label, left, right, back;

  unsigned char alpha = (is_button ? 255 : 100);
  int depth = (is_button ? Config::action_button_depth : Config::label_depth);

  bool out = true;
  if (group)
  {
    out = false;
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

    if (open_left && arrow)
      left = set<C::Image>(id + "_left_circle:image", get<C::Image>("Goto_left:image"));
    else
    {
      left = set<C::Image>(id + "_left_circle:image", get<C::Image>("Left_circle:image"));
      left->set_alpha(alpha);
    }
    group->add(left);

    left->on() = true;
    left->set_relative_origin(1, 0.5);
    left->set_scale(scale);
    left->z() = depth - 1;
    left->set_collision(collision);

    if (open_right && arrow)
      right = set<C::Image>(id + "_right_circle:image", get<C::Image>("Goto_right:image"));
    else
    {
      right = set<C::Image>(id + "_right_circle:image", get<C::Image>("Right_circle:image"));
      right->set_alpha(alpha);
    }
    group->add(right);

    right->on() = true;
    right->set_relative_origin(0, 0.5);
    right->set_scale(scale);
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
      if (scale != 1.0)
      {
        // Avoid artefacts with bad divisions
        int factor = round(1. / (1. - scale));
        while (width % factor != 0)
          ++ width;
      }

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
    left->on() = !open_left || arrow;
  if (right)
    right->on() = !open_right || arrow;

  int half_width_minus = 0;
  int half_width_plus = 0;
  if (back)
  {
    int back_width = round (scale * back->width());
    half_width_minus = back_width / 2;
    half_width_plus = back_width - half_width_minus;
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

  return out;
}

void Interface::generate_action (const std::string& id, const std::string& action,
                                 const Button_orientation& orientation, const std::string& button,
                                 Point position, const Animation_style& style)
{
  auto label = request<C::String>(id + "_" + action + ":label");
  if (isupper(action[0]))
    label = get<C::String>(action + ":text");
  if (!label)
    label = get<C::String>("Default_" + action + ":label");
  if (id == "Default" && action == "inventory")
    label = get<C::String>("Inventory:label");

  bool open_left = false, open_right = false;
  if (position == Point())
    position = get<C::Position>(CURSOR__POSITION)->value();
  Point label_position;
  Point button_position;
  Point start_position;

  // Default cross orientation
  if (orientation == UP)
  {
    label_position = position + Vector(0, -80);
    button_position = position + Vector(0, -40);
    start_position = position + Vector(0, -14);
  }
  else if (orientation == DOWN)
  {
    label_position = position + Vector(0, 80);
    button_position = position + Vector(0, 40);
    start_position = position + Vector(0, 14);
  }
  else if (orientation == RIGHT_BUTTON)
  {
    label_position = position + Vector(40, 0);
    button_position = position + Vector(40, 0);
    start_position = position + Vector(14, 0);
    open_left = true;
  }
  else if (orientation == LEFT_BUTTON)
  {
    label_position = position + Vector(-40, 0);
    button_position = position + Vector(-40, 0);
    start_position = position + Vector(-14, 0);
    open_right = true;
  }

  // Side orientations
  else if (orientation == UPPER)
  {
    label_position = position + Vector(0, -100);
    button_position = position + Vector(0, -60);
    start_position = position + Vector(0, -14);
  }
  else if (orientation == DOWNER)
  {
    label_position = position + Vector(0, 100);
    button_position = position + Vector(0, 60);
    start_position = position + Vector(0, 14);
  }
  else if (orientation == UP_RIGHT)
  {
    label_position = position + Vector(50, -28.25);
    button_position = position + Vector(50, -28.25);
    start_position = position + Vector(-14, 0);
    open_left = true;
  }
  else if (orientation == UP_LEFT)
  {
    label_position = position + Vector(-50, -28.25);
    button_position = position + Vector(-50, -28.25);
    start_position = position + Vector(-14, 0);
    open_right = true;
  }
  else if (orientation == DOWN_RIGHT)
  {
    label_position = position + Vector(50, 28.25);
    button_position = position + Vector(50, 28.25);
    start_position = position + Vector(14, 0);
    open_left = true;
  }
  else if (orientation == DOWN_LEFT)
  {
    label_position = position + Vector(-50, 28.25);
    button_position = position + Vector(-50, 28.25);
    start_position = position + Vector(14, 0);
    open_right = true;
  }

  if (style != DEPLOY)
    start_position = button_position;

  if (id != "")
  {
    std::string label_id = id + "_" + action + "_label";
    update_label (false, label_id, locale(label->value()), open_left, open_right,
                  set<C::Absolute_position>(label_id + ":global_position", label_position), BOX);

    // UPPER and DOWNER configs might need to be moved to be on screen
    if (orientation == UPPER || orientation == DOWNER)
    {
      int diff = 0;
      auto lpos = get<C::Position>(label_id + "_left_circle:position");
      if (lpos->value().x() < Config::label_height)
        diff = Config::label_height - lpos->value().x();
      else
      {
        auto rpos = get<C::Position>(label_id + "_right_circle:position");
        if (rpos->value().x() > Config::world_width - Config::label_height)
          diff = lpos->value().x() - (Config::world_width - Config::label_height);
      }
      if (diff != 0)
      {
        auto pos = get<C::Position>(label_id + ":global_position");
        pos->set (Point (pos->value().x() + diff, pos->value().y()));
      }
    }

    animate_label (label_id, style);
  }

  std::string button_id = "";
  if (id == "")
  {
    button_id = "Default_" + action + "_button";
    update_label (true, button_id, button, false, false,
                  set<C::Absolute_position>(button_id + ":global_position", start_position), BOX);
    if (auto img = request<C::Image>("Default_" + action + "_button:image"))
      img->on() = false;
    get<C::Image>("Default_" + action + "_button_left_circle:image")->set_alpha(128);
    get<C::Image>("Default_" + action + "_button_right_circle:image")->set_alpha(128);
  }
  else
  {
    button_id = id + "_" + action + "_button";
    update_label (true, button_id, button, false, false,
                  set<C::Absolute_position>(button_id + ":global_position", start_position), BOX);
  }

  animate_label (button_id, style, true, button_position);
}

void Interface::animate_label (const std::string& id, const Animation_style& style,
                               bool button , const Point& position)
{
  if (style == NONE)
    return;

  double current_time = get<C::Double>(CLOCK__TIME)->value();

  if (!button)
  {
    if (style == ZOOM)
    {
      double from = 0.5 * 0.75, to = 0.5;
      if (get<C::Image>(id + ":image")->scale() != 0.5)
        std::swap(from, to);

      set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + ":image"), from, to, 255, 255);
    }
    else
    {
      unsigned char alpha = get<C::Image>(id + "_left_circle:image")->alpha();
      double scale = get<C::Image>(id + "_left_circle:image")->scale();
      set<C::GUI_image_animation>(id + "_back:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_back:image"), scale, scale, 0, alpha);
      set<C::GUI_image_animation>(id + "_left_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_left_circle:image"), scale, scale, 0, alpha);
      set<C::GUI_image_animation>(id + "_right_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_right_circle:image"), scale, scale, 0, alpha);
      if (style == DEPLOY)
        set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id + ":image"), 0.05, 0.5 * scale, 0, 255);
      else if (style == FADE || style == FADE_LABEL_ONLY)
        set<C::GUI_image_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                    get<C::Image>(id + ":image"), 0.5 * scale, 0.5 * scale, 0, 255);
    }
  }
  else
  {
    unsigned char alpha = get<C::Image>(id + "_left_circle:image")->alpha();
    if (style == DEPLOY)
    {
      set<C::GUI_position_animation>(id + ":animation", current_time, current_time + Config::inventory_speed,
                                     get<C::Position>(id + ":global_position"), position);
      set<C::GUI_image_animation>(id + "_left_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_left_circle:image"), 0.357, 1, alpha, alpha);
      set<C::GUI_image_animation>(id + "_right_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_right_circle:image"), 0.357, 1, alpha, alpha);
    }
    else if (style == FADE)
    {
      set<C::GUI_image_animation>(id + "_left_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_left_circle:image"), 1, 1, 0, alpha);
      set<C::GUI_image_animation>(id + "_right_circle:animation", current_time, current_time + Config::inventory_speed,
                                  get<C::Image>(id + "_right_circle:image"), 1, 1, 0, alpha);
    }
  }
}

void Interface:: update_inventory ()
{
  Input_mode mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  auto inventory_origin = get<C::Absolute_position>("Inventory:origin");

  double target = 0;
  if (status()->value() == IN_INVENTORY || status()->value() == OBJECT_CHOICE || status()->value() == INVENTORY_ACTION_CHOICE)
    target = Config::world_height - Config::inventory_height;
  else if ((mode == MOUSE || mode == TOUCHSCREEN) && status()->value() == IDLE)
    target = Config::world_height;
  else
    target = Config::inventory_active_zone + Config::world_height; // hidden t the bottom

  if (target != inventory_origin->value().y() && !request<C::GUI_animation>("Inventory:animation"))
  {
    double current_time = get<C::Double>(CLOCK__TIME)->value();
    auto position = get<C::Position>("Inventory:origin");
    set<C::GUI_position_animation> ("Inventory:animation", current_time, current_time + Config::inventory_speed,
                                    position, Point(0, target));
  }

  auto inventory = get<C::Inventory>("Game:inventory");

  constexpr int inventory_margin = 100;
  constexpr int inventory_width = Config::world_width - inventory_margin * 2;

  std::size_t position = inventory->position();
  for (std::size_t i = 0; i < inventory->size(); ++ i)
  {
    auto img = get<C::Image>(inventory->get(i) + ":image");
    double factor = 0.7;
    unsigned char alpha = 255;
    unsigned char highlight = 0;

    if (status()->value() != INVENTORY_ACTION_CHOICE &&
        (img == m_collision || inventory->get(i) == m_active_object))
    {
      highlight = 255;
      factor = 0.9;

      if (mode != TOUCHSCREEN)
      {
        auto name = get<C::String>(img->entity() + ":name");
        auto position = get<C::Position>(img->entity() + ":position");

        Point p = position->value() + Vector(0, -Config::inventory_height / 2 - 2 * Config::inventory_margin);
        update_label(false, img->entity() + "_label", locale(name->value()), false, false,
                     set<C::Absolute_position>(img->entity() + "_label:global_position", p), UNCLICKABLE);
      }
    }
    if (inventory->get(i) == m_source)
    {
      highlight = 0;
      alpha = 128;
      factor = 0.5;
    }

    if (position <= i && i < position + Config::displayed_inventory_size)
    {
      std::size_t pos = i - position;
      double relative_pos = (1 + pos) / double(Config::displayed_inventory_size + 1);
      img->on() = true;
      img->set_scale (factor);
      img->set_alpha (alpha);
      img->set_highlight(highlight);

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
  if (status()->value() != DIALOG_CHOICE)
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
                        locale(choices[std::size_t(c)]));
      img_off->z() = Config::dialog_depth;
      img_off->set_scale(0.75);
      img_off->set_relative_origin(0., 1.);

      auto img_on
        = set<C::Image>(entity + "_on:image", interface_font,
                        get<C::String>(player + ":color")->value(),
                        locale(choices[std::size_t(c)]));
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

  if (get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value() == GAMEPAD)
  {
    for (int c = int(choices.size()) - 1; c >= 0; -- c)
    {
      std::string entity = "Dialog_choice_" + std::to_string(c);
      auto img_off = get<C::Image>(entity + "_off:image");
      auto img_on = get<C::Image>(entity + "_on:image");

      bool on = (entity == m_active_object);
      img_off->on() = !on;
      img_on->on() = on;
    }
  }
  else
  {
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
}

void Interface::generate_code_hover()
{
  const std::string& player = get<C::String>("Player:name")->value();
  auto code = get<C::Code>("Game:code");
  auto window = get<C::Image>("Game:window");
  auto position
    = get<C::Position>(window->entity() + ":position");

  const std::string& color_str = get<C::String>(player + ":color")->value();
  RGB_color color = color_from_string (color_str);
  auto img = set<C::Image>("Code_hover:image", code->xmax() - code->xmin(), code->ymax() - code->ymin(),
                           color[0], color[1], color[2], 128);
  img->set_collision(UNCLICKABLE);
  img->z() = Config::inventory_depth;
  set<C::Absolute_position>
      ("Code_hover:position", Point(code->xmin(), code->ymin())
       + Vector(position->value())
       - Vector (0.5  * window->width(),
                 0.5 * window->height()));

}
#endif

} // namespace Sosage::System
