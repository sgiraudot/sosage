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
#include <Sosage/Utils/conversions.h>

#include <queue>

namespace Sosage::System
{

namespace C = Component;

Interface::Interface (Content& content)
  : Base (content)
  , m_selector_type (NO_SEL)
{

}

void Interface::run()
{
  SOSAGE_TIMER_START(System_Interface__run);
  SOSAGE_UPDATE_DBG_LOCATION("Interface::run()");

  update_inventory();
  update_active_objects();
  update_action_selector();
  update_notifications();
  update_object_switcher();
  update_code_hover();
  update_dialog_choices();
  update_cursor();
  update_menu();
  SOSAGE_TIMER_STOP(System_Interface__run);
}

void Interface::init()
{
  // Init black screen
  auto blackscreen = set<C::Image>("Blackscreen", "image",
                                   Config::world_width,
                                   Config::world_height,
                                   0, 0, 0, 255);
  blackscreen->on() = false;
  blackscreen->z() = Config::overlay_depth;
  blackscreen->set_collision(UNCLICKABLE);

  set<C::Absolute_position>("Blackscreen", "position", Point(0,0));

  // Init inventory (keep existing value if it's a re-init after language change)
  auto inventory_origin = get_or_set<C::Absolute_position>
                          ("Inventory", "origin",
                           Point(0, Config::world_height + Config::inventory_active_zone));

  auto inventory_label = set<C::Image>("Inventory_label", "image", get<C::Font>("Interface", "font"),
                                       "FFFFFF", locale_get("Inventory", "label"));
  inventory_label->set_collision(UNCLICKABLE);
  inventory_label->z() = Config::inventory_depth;
  inventory_label->set_scale(0.5);
  inventory_label->set_relative_origin (0.5, 0.5);

  auto inventory_label_background = set<C::Image>("Inventory_label_background", "image",
                                                  Config::label_margin + inventory_label->width() / 2,
                                                  Config::label_height);
  inventory_label_background->set_collision(BOX);
  inventory_label_background->z() = Config::interface_depth;
  int label_width = inventory_label_background->width();
  int label_height = inventory_label_background->height();
  set<C::Relative_position>("Inventory_label_background", "position", inventory_origin, Vector(0, -label_height));
  set<C::Relative_position>("Chamfer", "position", inventory_origin, Vector(label_width, -label_height));
  get<C::Image>("Chamfer", "image")->set_collision(BOX);
  set<C::Relative_position>("Inventory_label", "position", inventory_origin, Vector(label_width / 2, - 0.5 * label_height));

  auto inventory_background = set<C::Image>("Inventory_background", "image", Config::world_width, Config::inventory_height);
  inventory_background->z() = Config::interface_depth;
  inventory_background->set_collision(UNCLICKABLE);
  set<C::Relative_position>("Inventory_background", "position", inventory_origin);

  set<C::Relative_position>("Left_arrow", "position", inventory_origin, Vector(Config::inventory_margin, Config::inventory_height / 2));
  set<C::Relative_position>("Right_arrow", "position", inventory_origin, Vector(Config::world_width - Config::inventory_margin, Config::inventory_height / 2));

  auto switcher_pos = wriggly_position
                      ("Switcher", "position",
                       C::make_handle<C::Absolute_position>("Switcher", "position",
                                                            Point(Config::label_height / 2,
                                                                  Config::world_height -
                                                                  Config::label_height / 2)),
                      Vector(), UP, true, true);

  // Init object switchers
  create_label ("Keyboard_switcher_left", "Tab", LABEL_BUTTON, UNCLICKABLE);
  auto kb_left_pos = set<C::Relative_position>("Keyboard_switcher_left", "global_position", switcher_pos);
  update_label ("Keyboard_switcher_left", LABEL_BUTTON, kb_left_pos);
  auto kb_left_back = get<C::Image>("Keyboard_switcher_left_back", "image");
  kb_left_pos->set (Point (switcher_pos->value().x() + kb_left_back->width() / 2,
                           switcher_pos->value().y() - kb_left_back->height() / 2));

  create_label ("Gamepad_switcher_left", "L", LABEL_BUTTON, UNCLICKABLE);
  auto left_pos = set<C::Relative_position>("Gamepad_switcher_left", "global_position", switcher_pos);
  update_label ("Gamepad_switcher_left", LABEL_BUTTON, left_pos);
  auto left_back = get<C::Image>("Gamepad_switcher_left_back", "image");
  left_pos->set (Point (switcher_pos->value().x() + left_back->width() / 2,
                        switcher_pos->value().y() - left_back->height() / 2));

  create_label ("Keyboard_switcher_label", locale_get("Switch_target", "text"), OPEN_LEFT, UNCLICKABLE);
  auto kb_img = get<C::Image>("Keyboard_switcher_label_back", "image");
  auto kb_pos = set<C::Relative_position>("Keyboard_switcher_label", "global_position", switcher_pos);
  update_label ("Keyboard_switcher_label", OPEN_LEFT, kb_pos);
  kb_pos->set (Point (value<C::Position>("Keyboard_switcher_left_back", "position").x() + kb_img->width() / 2,
                      kb_left_pos->value().y()));
  get<C::Relative_position>("Keyboard_switcher_label", "position")->set(Vector(Config::label_margin,0));
  get<C::Relative_position>("Keyboard_switcher_label_back", "position")->set(Vector(0,0));

  create_label ("Gamepad_switcher_label", locale_get("Switch_target", "text"), OPEN, UNCLICKABLE);
  auto img = get<C::Image>("Gamepad_switcher_label_back", "image");
  auto pos = set<C::Relative_position>("Gamepad_switcher_label", "global_position", switcher_pos);
  update_label ("Gamepad_switcher_label", OPEN, pos);
  pos->set (Point (value<C::Position>("Gamepad_switcher_left_back", "position").x() + img->width() / 2,
                   left_pos->value().y()));

  create_label ("Gamepad_switcher_right", "R", LABEL_BUTTON, UNCLICKABLE);
  auto right_pos = set<C::Relative_position>("Gamepad_switcher_right", "global_position", switcher_pos);
  update_label ("Gamepad_switcher_right", LABEL_BUTTON, right_pos);
  right_pos->set (Point (pos->value().x() + img->width() / 2, left_pos->value().y()));

  auto kb_switcher = set<C::Group>("Keyboard_switcher", "group");
  kb_switcher->add (get<C::Group>("Keyboard_switcher_left", "group"));
  kb_switcher->add (get<C::Group>("Keyboard_switcher_label", "group"));
  kb_switcher->apply<C::Image> ([&](auto img) { img->set_alpha(0); });

  auto gamepad_switcher = set<C::Group>("Gamepad_switcher", "group");
  gamepad_switcher->add (get<C::Group>("Gamepad_switcher_left", "group"));
  gamepad_switcher->add (get<C::Group>("Gamepad_switcher_label", "group"));
  gamepad_switcher->add (get<C::Group>("Gamepad_switcher_right", "group"));
  gamepad_switcher->apply<C::Image> ([&](auto img) { img->set_alpha(0); });

  // Init gamepad action selector position
  set<C::Absolute_position>("Gamepad_action_selector", "position",
                            inventory_origin->value() + Vector (Config::world_width - 260, -Config::inventory_active_zone - 130));

  set<C::Variable>("Selected_object", "position", get<C::Position>(CURSOR__POSITION));

  init_menus();
}

void Interface::update_active_objects()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_active_objects()");
  // Clear if input mode changed
  if (receive("Input_mode", "changed") || status()->is (IN_MENU))
  {
    if (!m_active_objects.empty())
    {
      for (const std::string& a : m_active_objects)
      {
        highlight_object (a, 0);
        delete_label (a + "_label");
      }
      m_active_objects.clear();
      m_active_object = "";
    }
    else if (m_active_object != "")
    {
      highlight_object (m_active_object, 0);
      delete_label (m_active_object + "_label");
      m_active_object = "";
    }
    return;
  }

  if (auto active_objects = request<C::Vector<std::string>>("Interface", "active_objects"))
  {
    if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD)
    {
      auto active_object = get<C::String>("Interface", "active_object");

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

      for (const std::string& a : all_active)
      {
        // Object was active and is not anymore
        if (!contains(new_active, a))
        {
          highlight_object (a, 0);
          delete_label (a + "_label");
        }
        // Object was selected and is not anymore (but still here)
        else if (a == old_selected && a != new_selected)
        {
          update_label_position(a, 0.75);
          animate_label(a + "_label", ZOOM);
          highlight_object (a, 192);
        }
        // Object was not active and is now active
        else if (!contains(old_active, a) && contains(new_active, a))
        {
          create_object_label (a);
          highlight_object (a, (a == new_selected ? 255 : 192));
        }
        // Object was already here but is now selected
        else if (a != old_selected && a == new_selected)
        {
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

      for (const std::string& a : all_active)
      {
        // Object was active and is not anymore
        if (!contains(new_active, a))
        {
          highlight_object (a, 0);
          delete_label (a + "_label");
        }
        // Object was not active and is now active
        else if (!contains(old_active, a) && contains(new_active, a))
        {
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
  else if (auto active = request<C::String>("Interface", "active_object"))
  {
    if (status()->is (INVENTORY_ACTION_CHOICE))
    {
      if (m_active_object != "")
      {
        highlight_object (m_active_object, 0);
        delete_label (m_active_object + "_label");
      }
    }
    // Active object didn't change, nothing to do
    else if (m_active_object == active->value())
    {}
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
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_action_selector()");

  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (status()->is(CUTSCENE, LOCKED, DIALOG_CHOICE))
  {
    if (m_action_selector[0] != "")
      reset_action_selector();
    return;
  }

  if (mode == GAMEPAD)
  {
    if (status()->is(IDLE))
    {
      double inactive_time = value<C::Double>(CLOCK__TIME) - value<C::Double>(CLOCK__LATEST_ACTIVE);
      if (auto first_idle = request<C::Double>("First_idle", "time"))
      {
        // Keep selector alive for the 30 first seconds of game
        if (value<C::Double>(CLOCK__TIME) - first_idle->value() < 30)
          inactive_time = 100;
        else
          remove (first_idle);
      }

      if (inactive_time > 5)
      {
        set_action_selector (GP_IDLE, value<C::String>("Interface", "active_object", ""));
        remove ("Player", "not_moved_yet", true);
      }
      else
        set_action_selector(NO_SEL);
    }
    else if (status()->is(IN_INVENTORY))
      set_action_selector (GP_INV_ACTION_SEL, value<C::String>("Interface", "active_object", ""));
    else if (status()->is(ACTION_CHOICE))
      set_action_selector(ACTION_SEL, value<C::String>("Interface", "action_choice_target"));
    else if (status()->is(OBJECT_CHOICE, IN_WINDOW, IN_CODE))
      set_action_selector (OKNOTOK);
    else if (status()->is(IN_MENU))
      set_action_selector(OKCONTINUE);
    else
      set_action_selector(NO_SEL);
  }
  else
  {
    if (status()->is(ACTION_CHOICE))
      set_action_selector(ACTION_SEL, value<C::String>("Interface", "action_choice_target"));
    else if (status()->is(INVENTORY_ACTION_CHOICE))
      set_action_selector(INV_ACTION_SEL, value<C::String>("Interface", "action_choice_target"));
    else
      set_action_selector(NO_SEL);

    if (m_action_selector[0] != "")
    {
      std::string active_button = "";
      if (auto a = request<C::String>("Interface", "active_button"))
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
        get<C::Image>(id + "_button_back", "image")->set_highlight(highlight);
      }
    }
  }
}

void Interface::update_object_switcher()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_object_switcher()");

#ifdef SOSAGE_DEV
  bool keyboard_on = false;
#endif
  bool gamepad_on = false;

  double inactive_time = value<C::Double>(CLOCK__TIME) - value<C::Double>(CLOCK__LATEST_ACTIVE);
  if (auto first_idle = request<C::Double>("First_idle", "time"))
  {
    // Keep switcher alive for the 30 first seconds of game
    if (value<C::Double>(CLOCK__TIME) - first_idle->value() < 30)
      inactive_time = 100;
    else
      remove (first_idle);
  }

  if (inactive_time >= 5)
  {
    const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);
    if (mode == GAMEPAD && !status()->is(IN_MENU))
      if (auto active_objects = request<C::Vector<std::string>>("Interface", "active_objects"))
        if (active_objects->value().size() > 1)
        {
#ifdef SOSAGE_DEV
          keyboard_on = (value<C::Simple<Gamepad_type>>(GAMEPAD__TYPE) == KEYBOARD);
          gamepad_on = !keyboard_on;
#else
          gamepad_on = true;
#endif
        }
  }

#if 0 // Just to test gamepad display using keyboard
  std::swap(keyboard_on, gamepad_on);
#endif

#ifdef SOSAGE_DEV
  if (keyboard_on)
  {
    auto img = get<C::Image>("Keyboard_switcher_left", "image");
    if (img->alpha() != 255)
    {
      img->on() = true;
      fade_action_selector ("Keyboard_switcher", true);
    }
  }
#endif
  if (gamepad_on)
  {
    auto img = get<C::Image>("Gamepad_switcher_left", "image");
    if (img->alpha() != 255)
    {
      img->on() = true;
      fade_action_selector ("Gamepad_switcher", true);
    }
  }
#ifdef SOSAGE_DEV
  if (!keyboard_on)
  {
    auto img = get<C::Image>("Keyboard_switcher_left", "image");
    if (img->alpha() != 0)
      fade_action_selector ("Keyboard_switcher", false);
    else
      img->on() = false;
  }
#endif
  if (!gamepad_on)
  {
    auto img = get<C::Image>("Gamepad_switcher_left", "image");
    if (img->alpha() != 0)
      fade_action_selector ("Gamepad_switcher", false);
    else
      img->on() = false;
  }
}

void Interface::update_notifications()
{
  bool in_new_room = signal("Game", "in_new_room");
  if (in_new_room)
    debug << "In new room ? " << in_new_room << std::endl;

  std::vector<C::Handle> to_remove;
  for (auto c : components("notification"))
  {
    const std::string& id = c->entity();

    if (receive (id, "end_notification") || in_new_room)
    {
      delete_label(id, !in_new_room);
      to_remove.push_back (c);
      continue;
    }

    // If notification was already created, ignore
    if (request<C::Image>(id, "image"))
      continue;

    auto group = set<C::Group>(id, "group");
    std::string text = C::cast<C::String>(c)->value();

    int number = to_int(std::string(id.begin() + id.find('_') + 1, id.end()));

    int depth = Config::notification_depth;
    unsigned int alpha = 170;

    auto label = set<C::Image>(id, "image", get<C::Font>("Interface", "font"),
                               "000000", text);
    label->set_relative_origin(0.5, 0.5);
    label->set_alpha(alpha);
    label->set_scale(0.5);
    label->z() = depth+1;
    label->set_collision(UNCLICKABLE);
    group->add(label);

    auto left = C::make_handle<C::Image>(id + "_left_circle", "image",
                                         get<C::Image>("White_left_circle", "image"));
    auto right = C::make_handle<C::Image>(id + "_right_circle", "image",
                                          get<C::Image>("White_right_circle", "image"));

    int width = label->width() / 2;
    auto back = C::make_handle<C::Image>(id + "_back", "image", width,
                              Config::label_height,
                              255, 255, 255);
    left->compose_with (back);
    back = left;
    back->compose_with (right);

    back->set_relative_origin(0, 0);
    back->z() = depth;
    back->set_collision(UNCLICKABLE);
    back->set_alpha(alpha);
    back->on() = true;
    back = set<C::Image>(id + "_back", "image", back);
    group->add(back);

    int y = Config::label_margin + number * (Config::label_margin + Config::label_height);

    set<C::Absolute_position>(id + "_back", "position",
                              Point (Config::label_margin, y));
    set<C::Absolute_position>(id, "position",
                              Point (Config::label_margin + back->width() / 2,
                                     y + back->height() / 2));
    animate_label(id, FADE);
  }

  for (auto tr : to_remove)
    remove(tr);
}

void Interface::update_inventory()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_inventory()");

  if (status()->is(IN_MENU))
    return;

  Input_mode mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  auto inventory_origin = get<C::Absolute_position>("Inventory", "origin");
  auto inventory = get<C::Inventory>("Game", "inventory");
  std::string active_object = value<C::String>("Interface", "active_object", "");
  std::string source_object = value<C::String>("Interface", "source_object", "");

  double target = 0;
  double as_target = 0;
  if (status()->is (IN_INVENTORY, OBJECT_CHOICE, INVENTORY_ACTION_CHOICE))
  {
    target = Config::world_height - Config::inventory_height;
    as_target = target - 80 - 2 * Config::inventory_margin;
    std::size_t position = inventory->position();
    for (std::size_t i = 0; i < inventory->size(); ++ i)
    {
      if (inventory->get(i) != active_object && inventory->get(i) != source_object)
        continue;

      if (position <= i && i < position + Config::displayed_inventory_size)
      {
        std::size_t pos = i - position;
        if (pos > 6)
          as_target -= 1.5 * Config::label_height;
      }
    }
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

  if (target != inventory_origin->value().y())
  {
    if (auto anim = request<C::GUI_position_animation>("Inventory", "animation"))
      anim->update(Point(0, target));
    else
    {
      double current_time = value<C::Double>(CLOCK__TIME);
      auto position = get<C::Position>("Inventory", "origin");
      set<C::GUI_position_animation> ("Inventory", "animation", current_time, current_time + Config::inventory_speed,
                                      position, Point(0, target));
    }
  }

  auto as_pos = get<C::Position>("Gamepad_action_selector", "position");
  if (as_target != as_pos->value().y())
  {
    if (auto as_anim = request<C::GUI_position_animation>("Gamepad_action_selector", "animation"))
      as_anim->update(Point(Config::world_width - 260, as_target));
    else
    {
      double current_time = value<C::Double>(CLOCK__TIME);
      auto as_pos = get<C::Position>("Gamepad_action_selector", "position");
      set<C::GUI_position_animation> ("Gamepad_action_selector", "animation", current_time, current_time + Config::inventory_speed,
                                      as_pos, Point(Config::world_width - 260, as_target));
    }
  }

  constexpr int inventory_margin = 100;
  constexpr int inventory_width = Config::world_width - inventory_margin * 2;


  std::size_t position = inventory->position();
  for (std::size_t i = 0; i < inventory->size(); ++ i)
  {
    auto img = get<C::Image>(inventory->get(i) , "image");
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

      set<C::Relative_position>(inventory->get(i) , "position", inventory_origin, Vector(x,y));
    }
    else
      img->on() = false;
  }

  get<C::Image>("Left_arrow", "image")->on() = (inventory->position() > 0);
  get<C::Image>("Right_arrow", "image")->on() = (inventory->size() - inventory->position()
                                              > Config::displayed_inventory_size);
}

void Interface::update_code_hover()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_code_hover()");

  double current_time = value<C::Double>(CLOCK__TIME);

  if (receive("Interface", "show_window"))
  {
    auto window = get<C::Image>("Game", "window");
    window->on() = true;
    set<C::GUI_image_animation>(window->entity() , "animation",
                                current_time, current_time + Config::inventory_speed,
                                window, 0, 1, 0, 255);
  }
  if (receive("Interface", "hide_window"))
  {
    auto window = get<C::Image>("Game", "window");
    set<C::GUI_image_animation>(window->entity() , "animation",
                                current_time, current_time + Config::inventory_speed,
                                window, 1, 0, 255, 0);
  }

  if (receive("Code", "hover"))
  {
    // Possible improvment: avoid creating image at each frame
    const std::string& player = value<C::String>("Player", "name");
    auto code = get<C::Code>("Game", "code");
    auto window = get<C::Image>("Game", "window");
    auto position
        = get<C::Position>(window->entity() , "position");

    const std::string& color_str = value<C::String>(player , "color");
    RGB_color color = color_from_string (color_str);
    auto img = set<C::Image>("Code_hover", "image", code->xmax() - code->xmin(), code->ymax() - code->ymin(),
                             color[0], color[1], color[2], 128);
    img->set_collision(UNCLICKABLE);
    img->z() = Config::inventory_depth;
    set<C::Absolute_position>
        ("Code_hover", "position", Point(code->xmin(), code->ymin())
         + Vector(position->value())
         - Vector (0.5  * window->width(),
                   0.5 * window->height()));
  }
  else if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == MOUSE)
  {
    remove("Code_hover", "image", true);
  }
}

void Interface::update_dialog_choices()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_dialog_choices()");

  if (receive("Dialog", "clean"))
  {
    // Optional, if user is really quick, choice can be
    // made *before* images are actually created
    if (request<C::Image>("Dialog_choice_background", "image"))
    {
      const std::vector<std::string>& choices
          = value<C::Vector<std::string> >("Dialog", "choices");

      // Clean up
      for (int c = int(choices.size()) - 1; c >= 0; -- c)
      {
        std::string entity = "Dialog_choice_" + std::to_string(c);
        remove(entity + "_off", "image");
        remove(entity + "_off", "position");
        remove(entity + "_on", "image");
        remove(entity + "_on", "position");
      }
      remove("Dialog_choice_background", "image");
      remove("Dialog_choice_background", "position");
    }
  }

  if (!status()->is(DIALOG_CHOICE) && !status()->was(DIALOG_CHOICE))
    return;

  const std::vector<std::string>& choices
      = value<C::Vector<std::string> >("Dialog", "choices");

  // Generate images if not done yet
  if (!request<C::Image>("Dialog_choice_background", "image"))
  {
    auto interface_font = get<C::Font> ("Dialog", "font");
    const std::string& player = value<C::String>("Player", "name");

    int bottom = Config::world_height;
    int y = bottom - 10;

    for (int c = int(choices.size()) - 1; c >= 0; -- c)
    {
      std::string entity = "Dialog_choice_" + std::to_string(c);
      auto img_off
        = set<C::Image>(entity + "_off", "image", interface_font, "FFFFFF",
                        locale(choices[std::size_t(c)]));
      img_off->z() = Config::dialog_depth;
      img_off->set_scale(0.75);
      img_off->set_relative_origin(0., 1.);

      auto img_on
        = set<C::Image>(entity + "_on", "image", interface_font,
                        value<C::String>(player , "color"),
                        locale(choices[std::size_t(c)]));
      img_on->z() = Config::dialog_depth;
      img_on->set_scale(0.75);
      img_on->set_relative_origin(0., 1.);
      y -= img_off->height() * 0.75;

    }

    auto background = set<C::Image> ("Dialog_choice_background", "image",
                                     Config::world_width, bottom - y + 20, 0, 0, 0, 192);
    background->set_relative_origin(0., 1.);
    set<C::Absolute_position>("Dialog_choice_background", "position", Point(0,bottom));
  }

  for (int c = int(choices.size()) - 1; c >= 0; -- c)
  {
    std::string entity = "Dialog_choice_" + std::to_string(c);
    get<C::Image>(entity + "_off", "image")->on() = status()->is(DIALOG_CHOICE);
    get<C::Image>(entity + "_on", "image")->on() = status()->is(DIALOG_CHOICE);
    get<C::Image> ("Dialog_choice_background", "image")->on() = status()->is(DIALOG_CHOICE);
  }

  if (status()->was(DIALOG_CHOICE))
    return;

  int bottom = Config::world_height;
  int y = bottom - 10;

  int choice = value<C::Int>("Interface", "active_dialog_item", -1);

  for (int c = int(choices.size()) - 1; c >= 0; -- c)
  {
    std::string entity = "Dialog_choice_" + std::to_string(c);
    auto img_off = get<C::Image>(entity + "_off", "image");
    auto img_on = get<C::Image>(entity + "_on", "image");

    Point p (10, y);
    set<C::Absolute_position>(entity + "_off", "position", p);
    set<C::Absolute_position>(entity + "_on", "position", p);
    y -= img_off->height() * 0.75;

    bool on = (c == choice);
    img_off->on() = !on;
    img_on->on() = on;
  }
}

void Interface::update_cursor()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_cursor()");

  const Input_mode& mode = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  auto state = get<C::String>("Cursor", "state");
  if (mode == MOUSE)
  {
    if (status()->is(IN_MENU))
      state->set("default");
    else if (auto source = request<C::String>("Interface", "source_object"))
    {
      if (state->value() != "selected")
      {
        state->set("selected");
        auto cursor_img = C::make_handle<C::Image>("Selected_object", "image",
                                                   get<C::Image>(source->value() , "image"));
        cursor_img->set_alpha(255);
        cursor_img->set_scale(0.28);
        cursor_img->set_collision(UNCLICKABLE);
        cursor_img->z() = Config::cursor_depth+1;

        auto cursor_cond = set<C::String_conditional>("Selected_object", "image", state);
        cursor_cond->add("selected", cursor_img);
      }
    }
    else
    {
      if (state->value() == "selected")
        remove("Selected_object", "image", true);

      if (!status()->is(INVENTORY_ACTION_CHOICE) && m_active_object != "")
      {
        if (auto right = request<C::Boolean>(m_active_object + "_goto", "right"))
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
  else
  {
    if (signal("Fake_touchscreen", "enabled"))
      state->set("fake_touchscreen");
    else
      state->set("none");
    remove("Selected_object", "image", true);
  }
}

} // namespace Sosage::System
