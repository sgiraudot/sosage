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

  if (status()->value() == PAUSED)
    return;

  auto mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE);

  if (status()->value() != CUTSCENE && status()->value() != LOCKED)
  {
    if (mode->value() == MOUSE || mode->value() == TOUCHSCREEN)
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
    else // if (mode->value() == GAMEPAD)
    {
      bool active_objects_changed = false;

      if (status()->value() == IDLE)
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

        if (status()->value() == IN_WINDOW)
          ;
        else if (status()->value() == IN_CODE)
          ;
        else if (status()->value() == IN_MENU)
          ;
        else if (status()->value() == DIALOG_CHOICE)
          ;
        else if (status()->value() == ACTION_CHOICE || status()->value() == INVENTORY_ACTION_CHOICE)
          ;
        else if (status()->value() == OBJECT_CHOICE)
          ;
        else if (status()->value() == IN_INVENTORY)
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


void Interface:: update_inventory ()
{
  Input_mode mode = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value();

  auto inventory_origin = get<C::Absolute_position>("Inventory:origin");
  if (status()->value() == IN_INVENTORY || status()->value() == OBJECT_CHOICE || status()->value() == INVENTORY_ACTION_CHOICE)
    inventory_origin->set (Point (0, Config::world_height - Config::inventory_height));
  else if ((mode == MOUSE || mode == TOUCHSCREEN) && status()->value() == IDLE)
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




} // namespace Sosage::System
