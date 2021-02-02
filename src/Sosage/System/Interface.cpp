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
      else
      {
        if (m_collision->entity().find("Verb_") == 0)
          verb_clicked();
        else if (m_collision->entity().find("Inventory_arrow") == 0)
          arrow_clicked();
        else
        {
          action_clicked("goto");
        }
      }
    }
  }
  update_action();
  // TO UNCOMMENT update_inventory();
  update_dialog_choices();
}

void Interface::init()
{
  auto pause_screen_pos
    = set<C::Position>("Pause_screen:position", Point(0, 0));
  set<C::Variable>("Window_overlay:position", pause_screen_pos);

  auto blackscreen = set<C::Image>("Blackscreen:image",
                                   Config::world_width,
                                   Config::world_height,
                                   0, 0, 0, 255);
  blackscreen->on() = false;
  blackscreen->z() = Config::overlay_depth;
  blackscreen->set_collision(UNCLICKABLE);

  set<C::Position>("Blackscreen:position", Point(0,0));

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

void Interface::action_clicked(const std::string& verb)
{
  bool action_found = false;

  // If clicked target is an object
  const std::string& entity = m_collision->character_entity();
  if (request<C::String>(entity + ":name"))
  {
    // First try binary action
    if (verb == "use" || verb == "give")
    {
      // If source was already cicked
      if (auto source = request<C::String>("Action:source"))
      {
        // Don't use source on source
        if (source->value() == entity)
          return;

        // Find binary action
        auto action
          = request<C::Action> (source->value() + ":use_" + entity);
        debug("Request ", source->value(), ":use_" , entity);
        if (action)
        {
          set<C::Variable>("Character:action", action);
          action_found = true;
        }
      }
      // Else check if object can be source
      else
      {
        auto state
          = request<C::String>(entity + ":state");
        if (state && (state->value() == "inventory"))
        {
          // Check if unary action exists
          if (auto action = request<C::Action> (entity + ":use"))
          {
            set<C::Variable>("Character:action", action);
            action_found = true;
          }
          else // Set object as source
          {
            set<C::String>("Action:source", entity);
            return;
          }
        }
      }
    }

    remove ("Action:source", true);

    if (!action_found)
    {
      // Then try to get unary action
      auto action
        = request<C::Action> (entity + ":" + verb);
      if (action)
      {
        set<C::Variable>("Character:action", action);
        action_found = true;
      }
    }

    if (!action_found)
    {
      // Finally fallback to default action
      if (verb == "goto")
      {
        // If default action on inventory,  look
        auto state = request<C::String>(entity + ":state");
        if ((state && (state->value() == "inventory")) ||
            !get<C::Boolean>("Click:left")->value())
          set<C::Variable>("Character:action", request<C::Action> (entity + ":look"));
        // Else, goto
        else
        {
          set<C::Variable>("Cursor:target", m_collision);
          emit ("Cursor:clicked"); // Logic handles this click
        }
        return;
      }
      // If no action found, use default
      else
        set<C::Variable>
          ("Character:action",
           get<C::Action> ("Default:" + verb));
    }
  }
  else
  {
    remove ("Action:source", true);
    if (verb == "goto")
    {
      set<C::Variable>("Cursor:target", m_collision);
      emit ("Cursor:clicked"); // Logic handles this click
      return;
    }
  }

  get<C::Variable> ("Chosen_verb:text")
    ->set(get<C::String>("Verb_goto:text"));
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

  set<C::Position>("Pause_text:position", Point(Config::world_width / 2,
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

      if (!position->absolute())
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

  if (previous_collision != m_collision)
  {
    remove("Object_label:image", true);
    remove("Object_label_back:image", true);
    get<C::Image>("Right_circle:image")->on() = false;
  }

  if (m_collision)
  {
    if (auto name = request<C::String>(m_collision->entity() + ":name"))
    {
      get<C::Image>(m_collision->entity() + ":image")->set_highlight(128);
      get<C::String>("Cursor:state")->set("object");

      if (previous_collision != m_collision)
      {
        std::string name_str = name->value();
        name_str[0] = toupper(name_str[0]);
        auto img = set<C::Image>("Object_label:image", get<C::Font>("Interface:font"), "FFFFFF", name_str);
        img->set_relative_origin(0, 0.5);
        img->set_scale(0.5);
        img->z() = Config::cursor_depth;

        auto right_circle = get<C::Image>("Right_circle:image");
        right_circle->on() = true;
        right_circle->set_relative_origin(0, 0.5);
        right_circle->set_alpha(100);
        right_circle->z() = Config::cursor_depth - 1;
        right_circle->set_collision(UNCLICKABLE);

        auto img_back = set<C::Image>("Object_label_back:image",
                                      20 + 0.5 * img->width(), right_circle->height());
        img_back->set_relative_origin(0, 0.5);
        img_back->set_alpha(100);
        img_back->z() = Config::cursor_depth - 1;
        img_back->set_collision(UNCLICKABLE);
      }
      set<C::Position>("Object_label:position",
                       cursor->value() + Vector(30, 0));
      set<C::Position>("Object_label_back:position", cursor->value());
      set<C::Position>("Right_circle:position",
                       cursor->value() + Vector(get<C::Image>("Object_label_back:image")->width(), 0));
    }

  }
}

void Interface::update_action ()
{
  // TODO
}

void Interface::update_inventory ()
{
  // TODO
  Status status = get<C::Status>(GAME__STATUS)->value();
  get<C::Image> ("Window_overlay:image")->on() = (status == IN_WINDOW);
  bool inventory_on = (status == IDLE);

  auto background = get<C::Image>("Interface_inventory:image");

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
      img->on() = inventory_on;

      int height = img->height();
      int inv_height = background->height();
      int target_height = int(0.8 * inv_height);

      img->set_scale (factor * target_height / double(height));

      int x = inv_pos->value().X() + int(relative_pos * background->width());
      int y = inv_pos->value().Y() + background->height() / 2;

      set<C::Position>(inventory->get(i) + ":position", Point(x,y));
    }
    else
      img->on() = false;
  }

  bool left_on = inventory_on && (inventory->position() > 0);
  bool right_on = inventory_on && (inventory->size() - inventory->position() > Config::displayed_inventory_size);

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
    set<C::Position>("Dialog_choice_background:position", Point(0,bottom));
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
    set<C::Position>(entity + "_off:position", p);
    set<C::Position>(entity + "_on:position", p);
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
