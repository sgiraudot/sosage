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

  auto status = get<C::Status>(GAME__STATUS);
  if (status->value() == PAUSED)
    return;

  if (status->value() != CUTSCENE)
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
      else if (status->value() == ACTION_CHOICE)
        action_clicked();
      else
      {
        // Click on an object
        if (request<C::String>(m_collision->entity() + ":name"))
          generate_action_choice (m_collision->entity());
        else if (m_collision->entity().find("Inventory_arrow") == 0)
          arrow_clicked();
        else // Click anywhere else
        {
          emit ("Cursor:clicked"); // Logic handles this click
        }
      }
    }
  }

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

  auto inventory_label = set<C::Image>("Inventory_label:image", get<C::Font>("Interface:font"),
                                       "FFFFFF", get<C::String>("Inventory:label")->value());
  inventory_label->z() = Config::inventory_depth;
  inventory_label->set_scale(0.5);
  inventory_label->set_relative_origin (0.5, 0.5);

  auto inventory_label_background = set<C::Image>("Inventory_label_background:image",
                                                  Config::label_margin + inventory_label->width() / 2,
                                                  Config::label_height);
  inventory_label_background->z() = Config::interface_depth;
  int label_position
      = Config::world_height - inventory_label_background->height();
  set<C::Absolute_position>("Inventory_label_background:position",
                   Point(0, label_position));

  set<C::Absolute_position>("Chamfer:position", Point(inventory_label_background->width(),
                                             label_position));

  set<C::Absolute_position>("Inventory_label:position",
                   Point(inventory_label_background->width() / 2,
                         label_position + inventory_label_background->height() / 2));

  auto inventory_background = set<C::Image>("Inventory_background:image", Config::world_width, Config::inventory_height);
  inventory_background->on() = false;
  inventory_background->z() = Config::interface_depth;
  inventory_background->set_collision(UNCLICKABLE);
  set<C::Absolute_position>("Inventory_background:position", Point(0, Config::world_height - Config::inventory_height));

  set<C::Absolute_position>("Left_arrow:position", Point(Config::inventory_margin, Config::world_height - Config::inventory_height / 2));
  set<C::Absolute_position>("Right_arrow:position", Point(Config::world_width - Config::inventory_margin, Config::world_height - Config::inventory_height / 2));

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

void Interface::verb_clicked()
{
  get<C::Variable> ("Chosen_verb:text")
    ->set(get<C::String>(m_collision->entity() + ":text"));
  emit ("Click:play_sound");
  remove ("Action:source", true);
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
  get<C::Status>(GAME__STATUS)->pop();

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

  set<C::Variable>("Character:action", get<C::Action>(object_id + ":action"));
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
      get<C::String>("Cursor:state")->set("default");
    }
  }

  auto status = get<C::Status>(GAME__STATUS);

  if (status->value() == IDLE
      && cursor->value().y() > Config::world_height - Config::inventory_active_zone)
  {
    status->push (INVENTORY_CHOICE);
    return;
  }
  if (status->value() == INVENTORY_CHOICE
      && cursor->value().y() < Config::world_height - Config::inventory_height)
  {
    status->pop();
    return;
  }

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

  if (status->value() == IDLE &&
      (m_collision->id() == "Chamfer:image" ||
       m_collision->id() == "Inventory_label_background:image"))
  {
    status->push(INVENTORY_CHOICE);
    return;
  }

  if (status->value() != IDLE && status->value() != ACTION_CHOICE)
    return;
  bool object_mode = (status->value() == ACTION_CHOICE);
  if (previous_collision && (previous_collision != m_collision))
  {
    const std::string& id = previous_collision->entity();
    if (object_mode)
    {
      std::size_t pos = id.find("_button_");
      if (pos == std::string::npos)
        pos = id.find("_label");
      if (pos != std::string::npos)
      {
        std::string object_id (id.begin(), id.begin() + pos);
        get<C::Image>(object_id + "_button_left:image")->set_highlight(0);
        get<C::Image>(object_id + "_button_right:image")->set_highlight(0);
        get<C::Image>(object_id + "_button_left:image")->set_alpha(255);
        get<C::Image>(object_id + "_button_right:image")->set_alpha(255);
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
    if (object_mode)
    {
      std::size_t pos = id.find("_button_");
      if (pos == std::string::npos)
        pos = id.find("_label");
      if (pos != std::string::npos)
      {
        std::string object_id (id.begin(), id.begin() + pos);
        get<C::Image>(object_id + "_button_left:image")->set_highlight(255);
        get<C::Image>(object_id + "_button_right:image")->set_highlight(255);
        get<C::Image>(object_id + "_button_left:image")->set_alpha(192);
        get<C::Image>(object_id + "_button_right:image")->set_alpha(192);
      }
    }
    else
    {
      if (auto name = request<C::String>(id + ":name"))
      {
        get<C::Image>(m_collision->entity() + ":image")->set_highlight(128);
        get<C::String>("Cursor:state")->set("object");

        update_label(id, name->value(), true, false, cursor->value(), UNCLICKABLE);
        if (get<C::Position>(id + "_right_circle:position")->value().x() >
            Config::world_width - Config::label_height)
          update_label(id, name->value(), false, true, cursor->value(), UNCLICKABLE);

      }
    }
  }
}

void Interface::clear_action_ids()
{
  for (const std::string& id : m_action_ids)
    remove (id, true);
  m_action_ids.clear();
}

void Interface::update_label (const std::string& id, std::string name,
                              bool open_left, bool open_right, const Point& position,
                              const Collision_type& collision)
{
  auto label = request<C::Image>(id + "_label:image");

  C::Image_handle left, right, back;

  if (label)
  {
    left = request<C::Image>(id + "_left_circle:image");
    right = request<C::Image>(id + "_right_circle:image");
    back = get<C::Image>(id + "_label_back:image");
  }
  else
  {
    name[0] = toupper(name[0]);
    label = set<C::Image>(id + "_label:image", get<C::Font>("Interface:font"), "FFFFFF", name);
    m_action_ids.push_back (label->id());

    label->set_relative_origin(0.5, 0.5);
    label->set_scale(0.5);
    label->z() = Config::label_depth;
    label->set_collision(collision);

    left = set<C::Image>(id + "_left_circle:image", get<C::Image>("Left_circle:image"));
    m_action_ids.push_back (left->id());
    left->on() = true;
    left->set_relative_origin(1, 0.5);
    left->set_alpha(100);
    left->z() = Config::label_depth - 1;
    left->set_collision(collision);

    right = set<C::Image>(id + "_right_circle:image", get<C::Image>("Right_circle:image"));
    m_action_ids.push_back (right->id());
    right->on() = true;
    right->set_relative_origin(0, 0.5);
    right->set_alpha(100);
    right->z() = Config::label_depth - 1;
    right->set_collision(collision);

    back = set<C::Image>(id + "_label_back:image",
                         Config::label_margin + label->width() / 2,
                         Config::label_height);
    m_action_ids.push_back (back->id());
    back->set_relative_origin(0.5, 0.5);
    back->set_alpha(100);
    back->z() = Config::label_depth - 1;
    back->set_collision(collision);
  }

  left->on() = !open_left;
  right->on() = !open_right;

  int half_width_minus = back->width() / 2;
  int half_width_plus = back->width() - half_width_minus;

  std::cerr << id << ": " << back->width() << " -> " << position.x() + Config::label_diff + half_width_plus
            << " " << position.x() - Config::label_diff - half_width_minus << std::endl;
  if (open_left)
  {
    set<C::Absolute_position>(id + "_label:position", position + Vector(Config::label_diff + half_width_plus, 0));
    set<C::Absolute_position>(id + "_label_back:position", position + Vector(half_width_plus, 0));
  }
  else if (open_right)
  {
    set<C::Absolute_position>(id + "_label:position", position + Vector(-Config::label_diff - half_width_minus, 0));
    set<C::Absolute_position>(id + "_label_back:position", position + Vector(-half_width_minus, 0));
  }
  else
  {
    set<C::Absolute_position>(id + "_label:position", position);
    set<C::Absolute_position>(id + "_label_back:position", position);
  }

  if (left)
    set<C::Absolute_position>(id + "_left_circle:position",
                     get<C::Position>(id + "_label_back:position")->value() + Vector(-half_width_minus, 0));
  if (right)
    set<C::Absolute_position>(id + "_right_circle:position",
                     get<C::Position>(id + "_label_back:position")->value() + Vector(half_width_plus, 0));
}

void Interface::generate_action_choice (const std::string& id)
{
  clear_action_ids();

  generate_action (id, "secondary", Config::WEST);
  if (get<C::Position>(id + "_secondary_left_circle:position")->value().x() < Config::label_height)
  {
    generate_action (id, "primary", Config::NORTHER);
    generate_action (id, "secondary", Config::NORTH_EAST);
    generate_action (id, "look", Config::SOUTH_EAST);
    generate_action (id, "inventory", Config::SOUTHER);
  }
  else
  {
    generate_action (id, "look", Config::EAST);
    if (get<C::Position>(id + "_secondary_right_circle:position")->value().x()
        > Config::world_width - Config::label_height)
    {
      generate_action (id, "primary", Config::NORTHER);
      generate_action (id, "secondary", Config::NORTH_WEST);
      generate_action (id, "look", Config::SOUTH_WEST);
      generate_action (id, "inventory", Config::SOUTHER);

    }
    else
    {
      generate_action (id, "primary", Config::NORTH);
      generate_action (id, "inventory", Config::SOUTH);

    }
  }

  get<C::Status>(GAME__STATUS)->push(ACTION_CHOICE);
}

void Interface::generate_action (const std::string& id, const std::string& action,
                                 const Config::Orientation& orientation)
{
  auto label = get<C::String>(id + "_" + action + ":label");
  bool open_left = false, open_right = false;
  const Point& position = get<C::Position>(CURSOR__POSITION)->value();;
  Point label_position;
  Point button_position;

  // Default cross orientation
  if (orientation == Config::NORTH)
  {
    label_position = position + Vector(0, -80);
    button_position = position + Vector(0, -40);
  }
  else if (orientation == Config::SOUTH)
  {
    label_position = position + Vector(0, 80);
    button_position = position + Vector(0, 40);
  }
  else if (orientation == Config::EAST)
  {
    label_position = position + Vector(40, 0);
    button_position = position + Vector(40, 0);
    open_left = true;
  }
  else if (orientation == Config::WEST)
  {
    label_position = position + Vector(-40, 0);
    button_position = position + Vector(-40, 0);
    open_right = true;
  }

  // Side orientations
  else if (orientation == Config::NORTHER)
  {
    label_position = position + Vector(0, -100);
    button_position = position + Vector(0, -60);
  }
  else if (orientation == Config::SOUTHER)
  {
    label_position = position + Vector(0, 100);
    button_position = position + Vector(0, 60);
  }
  else if (orientation == Config::NORTH_EAST)
  {
    label_position = position + Vector(50, -28.25);
    button_position = position + Vector(50, -28.25);
    open_left = true;
  }
  else if (orientation == Config::NORTH_WEST)
  {
    label_position = position + Vector(-50, -28.25);
    button_position = position + Vector(-50, -28.25);
    open_right = true;
  }
  else if (orientation == Config::SOUTH_EAST)
  {
    label_position = position + Vector(50, 28.25);
    button_position = position + Vector(50, 28.25);
    open_left = true;
  }
  else if (orientation == Config::SOUTH_WEST)
  {
    label_position = position + Vector(-50, 28.25);
    button_position = position + Vector(-50, 28.25);
    open_right = true;
  }


  update_label (id + "_" + action, label->value(), open_left, open_right, label_position, BOX);

  // NORTHER and SOUTHER configs might need to be moved to be on screen
  if (orientation == Config::NORTHER || orientation == Config::SOUTHER)
  {
    int diff = 0;
    auto lpos = get<C::Position>(id + "_" + action + "_left_circle:position");
    if (lpos->value().x() < Config::label_height)
      diff = Config::label_height - lpos->value().x();
    else
    {
      auto rpos = get<C::Position>(id + "_" + action + "_right_circle:position");
      if (rpos->value().x() > Config::world_width - Config::label_height)
        diff = lpos->value().x() - (Config::world_width - Config::label_height);
    }
    if (diff != 0)
      for (const std::string& element : { "left_circle", "right_circle", "label", "label_back" })
      {
        auto pos = get<C::Position>(id + "_" + action + "_" + element + ":position");
        pos->set (Point (pos->value().x() + diff, pos->value().y()));
      }
  }

  auto left = set<C::Image>(id + "_" + action + "_button_left:image", get<C::Image>("Left_circle:image"));
  m_action_ids.push_back (left->id());
  left->on() = true;
  left->set_relative_origin(1, 0.5);
  left->set_alpha(255);
  left->z() = Config::action_button_depth;
  left->set_collision(BOX);

  auto right = set<C::Image>(id + "_" + action + "_button_right:image", get<C::Image>("Right_circle:image"));
  m_action_ids.push_back (right->id());
  right->on() = true;
  right->set_relative_origin(0, 0.5);
  right->set_alpha(255);
  right->z() = Config::action_button_depth;
  right->set_collision(BOX);

  set<C::Absolute_position>(id + "_" + action + "_button_left:position", button_position);
  set<C::Absolute_position>(id + "_" + action + "_button_right:position", button_position);
}

void Interface::update_inventory ()
{
  Status status = get<C::Status>(GAME__STATUS)->value();
  //get<C::Image> ("Window_overlay:image")->on() = (status == IN_WINDOW);

  bool images_on = false;
  int inventory_y = Config::world_height;
  if (status == IDLE)
    images_on = false; // TO change
  if (status == INVENTORY_CHOICE)
  {
    images_on = true;
    inventory_y = Config::world_height - Config::inventory_height;
  }

  auto background = get<C::Image>("Inventory_background:image");
  background->on() = images_on;
  return;

  auto inventory = get<C::Inventory>("Game:inventory");

  auto inv_pos = get<C::Position>("Interface_inventory:position");

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
      img->on() = images_on;

      int height = img->height();
      int inv_height = background->height();
      int target_height = int(0.8 * inv_height);

      img->set_scale (factor * target_height / double(height));

      int x = inv_pos->value().X() + int(relative_pos * background->width());
      int y = inv_pos->value().Y() + background->height() / 2;

      set<C::Absolute_position>(inventory->get(i) + ":position", Point(x,y));
    }
    else
      img->on() = false;
  }

  bool left_on = images_on && (inventory->position() > 0);
  bool right_on = images_on && (inventory->size() - inventory->position() > Config::displayed_inventory_size);

  get<C::Image> ("Inventory_arrow_0:image")->on() = false;
  get<C::Image> ("Inventory_arrow_background_0:image")->on() = false;
  get<C::Image> ("Inventory_arrow_1:image")->on() = left_on;
  get<C::Image> ("Inventory_arrow_background_1:image")->on() = left_on;
  get<C::Image> ("Inventory_arrow_2:image")->on() = false;
  get<C::Image> ("Inventory_arrow_background_2:image")->on() = false;
  get<C::Image> ("Inventory_arrow_3:image")->on() = right_on;
  get<C::Image> ("Inventory_arrow_background_3:image")->on() = right_on;
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
    auto interface_font = get<C::Font> ("Interface:font");
    const std::string& player = get<C::String>("Player:name")->value();

    int bottom
        = std::max(get<C::Position>("Interface_action:position")->value().Y()
                   + get<C::Image>("Interface_action:image")->height(),
                   get<C::Position>("Interface_verbs:position")->value().Y()
                   + get<C::Image>("Interface_verbs:image")->height());
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
                                     Config::world_width, bottom - y + 20, 0, 0, 0);
    background->set_relative_origin(0., 1.);
    set<C::Absolute_position>("Dialog_choice_background:position", Point(0,bottom));
  }

  int bottom
      = std::max(get<C::Position>("Interface_action:position")->value().Y()
                 + get<C::Image>("Interface_action:image")->height(),
                 get<C::Position>("Interface_verbs:position")->value().Y()
                 + get<C::Image>("Interface_verbs:image")->height());
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
