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
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Interface.h>

namespace Sosage::System
{

namespace C = Component;

Interface::Interface (Content& content)
  : Base (content)
  , m_layout (Config::AUTO)
{

}

void Interface::run()
{
  auto status = get<C::Status>(GAME__STATUS);
  if (status->value() == PAUSED || status->value() == LOADING)
    return;

  if (request<C::Event>("window:rescaled"))
    update_layout();

  auto cursor = get<C::Position>(CURSOR__POSITION);
  detect_collision (cursor);

  auto clicked
    = request<C::Event>("cursor:clicked");
  if (clicked && m_collision)
  {
    if (status->value() == IN_WINDOW)
      window_clicked();
    else if (status->value() == IN_CODE)
      code_clicked(cursor);
    else if (status->value() == DIALOG_CHOICE)
      dialog_clicked();
    else
    {
      if (m_collision->entity().find("verb_") == 0)
        verb_clicked();
      else if (m_collision->entity().find("inventory_arrow") == 0)
        arrow_clicked();
      else
      {
        std::string verb
          = get<C::String> ("chosen_verb:text")->entity();
        verb = std::string (verb.begin() + 5, verb.end());

        action_clicked(verb);
      }
    }
  }

  update_action();
  update_inventory();
  update_dialog_choices();

}

void Interface::init()
{
  auto interface_font = get<C::Font> ("interface:font");
  std::string color_str = get<C::String> ("interface:color")->value();

  for (const auto& verb : { std::make_pair (std::string("open"), std::string("Ouvrir")),
        std::make_pair (std::string("close"), std::string("Fermer")),
        std::make_pair (std::string("give"), std::string("Donner")),
        std::make_pair (std::string("take"), std::string("Prendre")),
        std::make_pair (std::string("look"), std::string("Regarder")),
        std::make_pair (std::string("talk"), std::string("Parler")),
        std::make_pair (std::string("use"), std::string("Utiliser")),
        std::make_pair (std::string("move"), std::string("Déplacer")) })
  {
    set<C::String> ("verb_" + verb.first + ":text", verb.second);
    auto verb_img
      = set<C::Image> ("verb_" + verb.first + ":image",
                                         interface_font, color_str, verb.second);
    set<C::Position> ("verb_" + verb.first + ":position",
                                        Point(0,0));
    m_verbs.push_back (verb_img);
    verb_img->set_relative_origin(0.5, 0.5);
  }

  auto verb_goto = set<C::String> ("verb_goto:text", "Aller vers");
  set<C::Variable>("chosen_verb:text", verb_goto);

  auto pause_screen_pos
    = set<C::Position>("pause_screen:position", Point(0, 0));
  set<C::Variable>("window_overlay:position", pause_screen_pos);

  update_layout();
}

void Interface::window_clicked()
{
  auto window = get<C::Image>("game:window");
  window->on() = false;
  get<C::Status>(GAME__STATUS)->pop();
  remove("cursor:clicked");
}

void Interface::code_clicked (C::Position_handle cursor)
{
  auto code = get<C::Code>("game:code");
  auto window = get<C::Image>("game:window");
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
    if (code->click(p.x(), p.y()))
      set<C::Event>("code:button_clicked");
  }

  remove("cursor:clicked");
}

void Interface::dialog_clicked ()
{
  if (m_collision->entity().find("dialog_choice_") == std::string::npos)
  {
    remove("cursor:clicked");
    return;
  }

  const std::vector<std::string>& choices
      = get<C::Vector<std::string> >("dialog:choices")->value();

  int choice
      = std::atoi(std::string(m_collision->id().begin() +
                              std::string("dialog_choice_").size(),
                              m_collision->id().begin() +
                              m_collision->id().find(':')).c_str());

  set<C::Int>("dialog:choice", choice);

  // Clean up
  for (int c = choices.size() - 1; c >= 0; -- c)
  {
    std::string entity = "dialog_choice_" + std::to_string(c);
    remove(entity + "_off:image");
    remove(entity + "_off:position");
    remove(entity + "_on:image");
    remove(entity + "_on:position");
  }
  remove("dialog_choice_background:image");
  remove("dialog_choice_background:position");

  get<C::Status>(GAME__STATUS)->pop();

  remove("cursor:clicked");
}

void Interface::verb_clicked()
{
  get<C::Variable> ("chosen_verb:text")
    ->set(get<C::String>(m_collision->entity() + ":text"));
  set<C::Event>("game:verb_clicked");
  remove("cursor:clicked");
  remove ("action:source", true);
}

void Interface::arrow_clicked()
{
  if (m_collision->entity().find("_0") != std::string::npos ||
      m_collision->entity().find("_1") != std::string::npos)
    get<C::Inventory>("game:inventory")->prev();
  else
    get<C::Inventory>("game:inventory")->next();
  remove("cursor:clicked");
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
      if (auto source = request<C::String>("action:source"))
      {
        // Don't use source on source
        if (source->value() == entity)
        {
          remove("cursor:clicked");
          return;
        }

        // Find binary action
        auto action
          = request<C::Action> (source->value() + ":use_" + entity);
        debug("Request " + source->value() + ":use_" + entity);
        if (action)
        {
          set<C::Variable>("character:action", action);
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
            set<C::Variable>("character:action", action);
            action_found = true;
          }
          else // Set object as source
          {
            set<C::String>("action:source", entity);
            remove("cursor:clicked");
            return;
          }
        }
      }
    }

    remove ("action:source", true);

    if (!action_found)
    {
      // Then try to get unary action
      auto action
        = request<C::Action> (entity + ":" + verb);
      if (action)
      {
        set<C::Variable>("character:action", action);
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
            !get<C::Boolean>("click:left")->value())
        {
          set<C::Variable>("character:action", request<C::Action> (entity + ":look"));
          remove("cursor:clicked");
        }
        // Else, goto
        else
          set<C::Variable>("cursor:target", m_collision);
        return;
      }
      // If no action found, use default
      else
        set<C::Variable>
          ("character:action",
           get<C::Action> ("default:" + verb));
    }
  }
  else
  {
    remove ("action:source", true);
    if (verb == "goto")
    {
      set<C::Variable>("cursor:target", m_collision);
      return;
    }
  }

  get<C::Variable> ("chosen_verb:text")
    ->set(get<C::String>("verb_goto:text"));

  remove("cursor:clicked");
}


void Interface::update_pause_screen()
{
  auto interface_font = get<C::Font> ("interface:font");

  auto pause_screen_img
    = C::make_handle<C::Image>
    ("pause_screen:image",
     Config::world_width + get<C::Int>("interface:width")->value(),
     Config::world_height + get<C::Int>("interface:height")->value(),
     0, 0, 0, 192);
  pause_screen_img->z() += 10;

  // Create pause screen
  auto status
    = get<C::Status>(GAME__STATUS);

  auto pause_screen
    = set<C::Conditional>("pause_screen:conditional",
                                            C::make_value_condition<Sosage::Status>(status, PAUSED),
                                            pause_screen_img);

  auto pause_text_img
    = C::make_handle<C::Image>("pause_text:image", interface_font, "FFFFFF", "PAUSE");
  pause_text_img->z() += 10;
  pause_text_img->set_relative_origin(0.5, 0.5);

  auto window_overlay_img
    = set<C::Image>("window_overlay:image",
                                      Config::world_width
                                      + get<C::Int>("interface:width")->value(),
                                      Config::world_height
                                      + get<C::Int>("interface:height")->value(),
                                      0, 0, 0, 128);
  window_overlay_img->z() = Config::interface_depth;
  window_overlay_img->on() = false;

  auto pause_text
    = set<C::Conditional>("pause_text:conditional",
                                            C::make_value_condition<Sosage::Status>(status, PAUSED),
                                            pause_text_img);

  set<C::Position>("pause_text:position", Point(Config::world_width / 2,
                                                                  Config::world_height / 2));

}

void Interface::detect_collision (C::Position_handle cursor)
{
  // Deactive previous collisions
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(m_verb_scale);
    if (auto name = request<C::String>(m_collision->entity() + ":name"))
      get<C::Image>("verb_look:image")->set_scale(m_verb_scale);
  }

  m_collision = C::Image_handle();
  double xcamera = get<C::Double>(CAMERA__POSITION)->value();

  const std::string& player = get<C::String>("player:name")->value();

  for (const auto& e : m_content)
    if (auto img = C::cast<C::Image>(e))
    {
      if (!img->on() ||
          img->collision() == UNCLICKABLE ||
          img->character_entity() == player ||
          img->id().find("debug") == 0 ||
          img->id().find("chosen_verb") == 0 ||
          img->id().find("interface_") == 0)
        continue;

      auto position = get<C::Position>(img->entity() + ":position");
      Point p = position->value();

      if (!position->absolute())
        p = p + Vector (-xcamera, 0);

      Point screen_position = p - img->core().scaling * Vector(img->origin());
      int xmin = screen_position.x();
      int ymin = screen_position.y();
      int xmax = xmin + (img->core().scaling * (img->xmax() - img->xmin()));
      int ymax = ymin + (img->core().scaling * (img->ymax() - img->ymin()));

      if (cursor->value().x() < xmin ||
          cursor->value().x() > xmax ||
          cursor->value().y() < ymin ||
          cursor->value().y() > ymax)
        continue;

      if (img->collision() == PIXEL_PERFECT)
      {
        int x_in_image = cursor->value().x() - xmin;
        int y_in_image = cursor->value().y() - ymin;
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

}

void Interface::update_action ()
{
  auto verb = get<C::String> ("chosen_verb:text");
  std::string target_object = "";

  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(1.1 * m_verb_scale);

    const std::string& entity = m_collision->character_entity();
    if (entity != get<C::String>("player:name")->value())
      if (auto name = request<C::String>(entity + ":name"))
      {
        get<C::Image>("verb_look:image")->set_scale(1.1 * m_verb_scale);
        target_object = name->value();
        auto state = request<C::String>(m_collision->entity() + ":state");
        if (state && state->value() == "inventory")
        {
          if (verb->entity() == "verb_goto")
            verb = get<C::String>("verb_look:text");
        }
      }
  }

  if (!request<C::Action>("character:action")
      || verb != get<C::String>("verb_goto:text"))
  {
    std::string text = verb->value() + " " + target_object;
    auto source = request<C::String>("action:source");
    if (source)
    {
      if (auto name
          = request<C::String>(source->value() + ":name"))
      {
        if (name->value() == target_object)
          target_object = "";
        text = verb->value() + " " + name->value() + " avec " + target_object;
      }
    }

    auto text_img
      = set<C::Image>("chosen_verb:image",
                                        get<C::Font>("interface:font"), "FFFFFF",
                                        text);

    text_img->set_relative_origin(0.5, 0.5);
    text_img->set_scale(0.8 * m_action_height / text_img->height());
  }
}

void Interface::update_inventory ()
{
  Status status = get<C::Status>(GAME__STATUS)->value();
  if (status == IN_WINDOW || status == LOCKED || status == DIALOG_CHOICE)
  {
    get<C::Image> ("interface_action:image")->z() = Config::inventory_over_depth;
    get<C::Image> ("interface_verbs:image")->z() = Config::inventory_over_depth;
    get<C::Image> ("interface_inventory:image")->z() = Config::inventory_over_depth;
    if (status == IN_WINDOW)
      get<C::Image> ("window_overlay:image")->on() = true;
    return;
  }
  else
  {
    get<C::Image> ("interface_action:image")->z() = Config::interface_depth;
    get<C::Image> ("interface_verbs:image")->z() = Config::interface_depth;
    get<C::Image> ("interface_inventory:image")->z() = Config::interface_depth;
    get<C::Image> ("window_overlay:image")->on() = false;
  }

  auto background = get<C::Image>("interface_inventory:image");

  auto inventory = get<C::Inventory>("game:inventory");

  auto inv_pos = get<C::Position>("interface_inventory:position");

  std::size_t position = inventory->position();
  for (std::size_t i = 0; i < inventory->size(); ++ i)
  {
    auto img = get<C::Image>(inventory->get(i) + ":image");
    double factor = 1.;

    if (img == m_collision)
      factor = 1.1;

    if (position <= i && i < position + Config::displayed_inventory_size)
    {
      std::size_t pos = i - position;
      double relative_pos = (1 + pos) / double(Config::displayed_inventory_size + 1);
      img->on() = true;

      int x, y;

      if (m_layout == Config::WIDESCREEN)
      {
        int width = img->width();
        int inv_width = background->width();
        int target_width = int(0.8 * inv_width);

        img->set_scale (factor * target_width / double(width));

        x = inv_pos->value().x() + background->width() / 2;
        y = inv_pos->value().y() + int(relative_pos * background->height());
      }
      else
      {
        int height = img->height();
        int inv_height = background->height();
        int target_height = int(0.8 * inv_height);

        img->set_scale (factor * target_height / double(height));

        x = inv_pos->value().x() + int(relative_pos * background->width());
        y = inv_pos->value().y() + background->height() / 2;
      }

      set<C::Position>(inventory->get(i) + ":position", Point(x,y));


    }
    else
      img->on() = false;
  }

  bool left_on = (inventory->position() > 0);
  bool right_on =  (inventory->size() - inventory->position() > Config::displayed_inventory_size);

  if (m_layout == Config::WIDESCREEN)
  {
    get<C::Image> ("inventory_arrow_0:image")->on() = left_on;
    get<C::Image> ("inventory_arrow_background_0:image")->on() = left_on;
    get<C::Image> ("inventory_arrow_1:image")->on() = false;
    get<C::Image> ("inventory_arrow_background_1:image")->on() = false;
    get<C::Image> ("inventory_arrow_2:image")->on() = right_on;
    get<C::Image> ("inventory_arrow_background_2:image")->on() = right_on;
    get<C::Image> ("inventory_arrow_3:image")->on() = false;
    get<C::Image> ("inventory_arrow_background_3:image")->on() = false;
  }
  else
  {
    get<C::Image> ("inventory_arrow_0:image")->on() = false;
    get<C::Image> ("inventory_arrow_background_0:image")->on() = false;
    get<C::Image> ("inventory_arrow_1:image")->on() = left_on;
    get<C::Image> ("inventory_arrow_background_1:image")->on() = left_on;
    get<C::Image> ("inventory_arrow_2:image")->on() = false;
    get<C::Image> ("inventory_arrow_background_2:image")->on() = false;
    get<C::Image> ("inventory_arrow_3:image")->on() = right_on;
    get<C::Image> ("inventory_arrow_background_3:image")->on() = right_on;
  }
}

void Interface::update_dialog_choices()
{
  if (get<C::Status>(GAME__STATUS)->value() != DIALOG_CHOICE)
    return;

  const std::vector<std::string>& choices
      = get<C::Vector<std::string> >("dialog:choices")->value();

  // Generate images if not done yet
  if (!request<C::Image>("dialog_choice_background:image"))
  {
    auto interface_font = get<C::Font> ("interface:font");
    const std::string& player = get<C::String>("player:name")->value();

    int bottom
        = std::max(get<C::Position>("interface_action:position")->value().y()
                   + get<C::Image>("interface_action:image")->height(),
                   get<C::Position>("interface_verbs:position")->value().y()
                   + get<C::Image>("interface_verbs:image")->height());
    int y = bottom - 10;

    for (int c = choices.size() - 1; c >= 0; -- c)
    {
      std::string entity = "dialog_choice_" + std::to_string(c);
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

      Point p (10, y);
      set<C::Position>(entity + "_off:position", p);
      set<C::Position>(entity + "_on:position", p);

      y -= img_off->height() * 0.75;
    }

    auto background = set<C::Image> ("dialog_choice_background:image",
                                     Config::world_width, bottom - y + 20, 0, 0, 0);
    background->set_relative_origin(0., 1.);
    set<C::Position>("dialog_choice_background:position", Point(0,bottom));
  }

  auto cursor = get<C::Position>(CURSOR__POSITION);

  for (int c = choices.size() - 1; c >= 0; -- c)
  {
    std::string entity = "dialog_choice_" + std::to_string(c);
    auto img_off = get<C::Image>(entity + "_off:image");
    auto img_on = get<C::Image>(entity + "_on:image");
    const Point& p = get<C::Position>(entity + "_off:position")->value();

    Point screen_position = p - img_off->core().scaling * Vector(img_off->origin());
    int xmin = screen_position.x();
    int ymin = screen_position.y();
    int xmax = xmin + (img_off->core().scaling * (img_off->xmax() - img_off->xmin()));
    int ymax = ymin + (img_off->core().scaling * (img_off->ymax() - img_off->ymin()));

    bool on = (xmin <= cursor->value().x() && cursor->value().x() <= xmax &&
               ymin <= cursor->value().y() && cursor->value().y() <= ymax);
    img_off->on() = !on;
    img_on->on() = on;
  }
}


} // namespace Sosage::System
