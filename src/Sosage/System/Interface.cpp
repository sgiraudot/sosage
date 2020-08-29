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

Interface::Interface (Content& content)
  : m_content (content)
  , m_layout (Config::AUTO)
{

}

void Interface::run()
{
  auto status = m_content.get<Component::Status>(GAME__STATUS);
  if (status->value() == PAUSED || status->value() == LOADING)
    return;

  if (m_content.request<Component::Event>("window:rescaled"))
    update_layout();

  auto cursor = m_content.get<Component::Position>(CURSOR__POSITION);
  detect_collision (cursor);

  auto clicked
    = m_content.request<Component::Event>("cursor:clicked");
  if (clicked && m_collision)
  {
    if (status->value() == IN_WINDOW)
      window_clicked();
    else if (status->value() == IN_CODE)
      code_clicked(cursor);
    else
    {
      if (m_collision->entity().find("verb_") == 0)
        verb_clicked();
      else if (m_collision->entity().find("inventory_arrow") == 0)
        arrow_clicked();
      else
      {
        std::string verb
          = m_content.get<Component::String> ("chosen_verb:text")->entity();
        verb = std::string (verb.begin() + 5, verb.end());

        action_clicked(verb);
      }
    }
  }

  update_action();
  update_inventory();

}

void Interface::init()
{
  auto interface_font = m_content.get<Component::Font> ("interface:font");
  std::string color_str = m_content.get<Component::String> ("interface:color")->value();

  for (const auto& verb : { std::make_pair (std::string("open"), std::string("Ouvrir")),
        std::make_pair (std::string("close"), std::string("Fermer")),
        std::make_pair (std::string("give"), std::string("Donner")),
        std::make_pair (std::string("take"), std::string("Prendre")),
        std::make_pair (std::string("look"), std::string("Regarder")),
        std::make_pair (std::string("talk"), std::string("Parler")),
        std::make_pair (std::string("use"), std::string("Utiliser")),
        std::make_pair (std::string("move"), std::string("Déplacer")) })
  {
    m_content.set<Component::String> ("verb_" + verb.first + ":text", verb.second);
    auto verb_img
      = m_content.set<Component::Image> ("verb_" + verb.first + ":image",
                                         interface_font, color_str, verb.second);
    m_content.set<Component::Position> ("verb_" + verb.first + ":position",
                                        Point(0,0));
    m_verbs.push_back (verb_img);
    verb_img->set_relative_origin(0.5, 0.5);
  }

  auto verb_goto = m_content.set<Component::String> ("verb_goto:text", "Aller vers");
  m_content.set<Component::Variable>("chosen_verb:text", verb_goto);

  auto pause_screen_pos
    = m_content.set<Component::Position>("pause_screen:position", Point(0, 0));
  m_content.set<Component::Variable>("window_overlay:position", pause_screen_pos);

  update_layout();
}

void Interface::window_clicked()
{
  auto window = m_content.get<Component::Image>("game:window");
  window->on() = false;
  m_content.get<Component::Status>(GAME__STATUS)->pop();
  m_content.remove("cursor:clicked");
}

void Interface::code_clicked (Component::Position_handle cursor)
{
  auto code = m_content.get<Component::Code>("game:code");
  auto window = m_content.get<Component::Image>("game:window");
  if (m_collision != window)
  {
    window->on() = false;
    code->reset();
    m_content.get<Component::Status>(GAME__STATUS)->pop();
  }
  else
  {
    auto position
      = m_content.get<Component::Position>(window->entity() + ":position");

    Point p = cursor->value() - Vector(position->value()) + Vector (0.5  * window->width(),
                                                                    0.5 * window->height());
    if (code->click(p.x(), p.y()))
    {
      m_content.set<Component::Event>("code:button_clicked");
      std::cerr << "Clicked!" << std::endl;
    }
  }

  m_content.remove("cursor:clicked");
}

void Interface::verb_clicked()
{
  m_content.get<Component::Variable> ("chosen_verb:text")
    ->set(m_content.get<Component::String>(m_collision->entity() + ":text"));
  m_content.set<Component::Event>("game:verb_clicked");
  m_content.remove("cursor:clicked");
  m_content.remove ("action:source", true);
}

void Interface::arrow_clicked()
{
  if (m_collision->entity().find("_0") != std::string::npos ||
      m_collision->entity().find("_1") != std::string::npos)
    m_content.get<Component::Inventory>("game:inventory")->prev();
  else
    m_content.get<Component::Inventory>("game:inventory")->next();
  m_content.remove("cursor:clicked");
}

void Interface::action_clicked(const std::string& verb)
{
  bool action_found = false;

  // If clicked target is an object
  const std::string& entity = m_collision->character_entity();
  if (m_content.request<Component::String>(entity + ":name"))
  {
    // First try binary action
    if (verb == "use" || verb == "give")
    {
      // If source was already cicked
      if (auto source = m_content.request<Component::String>("action:source"))
      {
        // Don't use source on source
        if (source->value() == entity)
        {
          m_content.remove("cursor:clicked");
          return;
        }

        // Find binary action
        auto action
          = m_content.request<Component::Action> (entity + ":use_"
                                                  + source->value());
        if (action)
        {
          m_content.set<Component::Variable>("character:action", action);
          action_found = true;
        }
      }
      // Else check if object can be source
      else
      {
        auto state
          = m_content.request<Component::String>(entity + ":state");
        if (state && (state->value() == "inventory"))
        {
          // Check if unary action exists
          if (auto action = m_content.request<Component::Action> (entity + ":use"))
          {
            m_content.set<Component::Variable>("character:action", action);
            action_found = true;
          }
          else // Set object as source
          {
            m_content.set<Component::String>("action:source", entity);
            m_content.remove("cursor:clicked");
            return;
          }
        }
      }
    }

    m_content.remove ("action:source", true);

    if (!action_found)
    {
      // Then try to get unary action
      auto action
        = m_content.request<Component::Action> (entity + ":" + verb);
      if (action)
      {
        m_content.set<Component::Variable>("character:action", action);
        action_found = true;
      }
    }

    if (!action_found)
    {
      // Finally fallback to default action
      if (verb == "goto")
      {
        // If default action on inventory,  look
        auto state = m_content.request<Component::String>(entity + ":state");
        if ((state && (state->value() == "inventory")) ||
            !m_content.get<Component::Boolean>("click:left")->value())
        {
          m_content.set<Component::Variable>("character:action", m_content.request<Component::Action> (entity + ":look"));
          m_content.remove("cursor:clicked");
        }
        // Else, goto
        else
          m_content.set<Component::Variable>("cursor:target", m_collision);
        return;
      }
      // If no action found, use default
      else
        m_content.set<Component::Variable>
          ("character:action",
           m_content.get<Component::Action> ("default:" + verb));
    }
  }
  else
  {
    m_content.remove ("action:source", true);
    if (verb == "goto")
    {
      m_content.set<Component::Variable>("cursor:target", m_collision);
      return;
    }
  }

  m_content.get<Component::Variable> ("chosen_verb:text")
    ->set(m_content.get<Component::String>("verb_goto:text"));

  m_content.remove("cursor:clicked");
}


void Interface::update_pause_screen()
{
  auto interface_font = m_content.get<Component::Font> ("interface:font");

  auto pause_screen_img
    = Component::make_handle<Component::Image>
    ("pause_screen:image",
     Config::world_width + m_content.get<Component::Int>("interface:width")->value(),
     Config::world_height + m_content.get<Component::Int>("interface:height")->value(),
     0, 0, 0, 192);
  pause_screen_img->z() += 10;

  // Create pause screen
  auto status
    = m_content.get<Component::Status>(GAME__STATUS);

  auto pause_screen
    = m_content.set<Component::Conditional>("pause_screen:conditional",
                                            Component::make_value_condition<Sosage::Status>(status, PAUSED),
                                            pause_screen_img);

  auto pause_text_img
    = Component::make_handle<Component::Image>("pause_text:image", interface_font, "FFFFFF", "PAUSE");
  pause_text_img->z() += 10;
  pause_text_img->set_relative_origin(0.5, 0.5);

  auto window_overlay_img
    = m_content.set<Component::Image>("window_overlay:image",
                                      Config::world_width
                                      + m_content.get<Component::Int>("interface:width")->value(),
                                      Config::world_height
                                      + m_content.get<Component::Int>("interface:height")->value(),
                                      0, 0, 0, 128);
  window_overlay_img->z() = Config::interface_depth;
  window_overlay_img->on() = false;

  auto pause_text
    = m_content.set<Component::Conditional>("pause_text:conditional",
                                            Component::make_value_condition<Sosage::Status>(status, PAUSED),
                                            pause_text_img);

  m_content.set<Component::Position>("pause_text:position", Point(Config::world_width / 2,
                                                                  Config::world_height / 2));

}

void Interface::detect_collision (Component::Position_handle cursor)
{
  // Deactive previous collisions
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(m_verb_scale);
    if (auto name = m_content.request<Component::String>(m_collision->entity() + ":name"))
      m_content.get<Component::Image>("verb_look:image")->set_scale(m_verb_scale);
  }

  m_collision = Component::Image_handle();
  double xcamera = m_content.get<Component::Double>(CAMERA__POSITION)->value();

  for (const auto& e : m_content)
    if (auto img = Component::cast<Component::Image>(e))
    {
      if (!img->on() ||
          img->collision() == UNCLICKABLE ||
          img->id().find("debug") == 0 ||
          img->id().find("chosen_verb") == 0 ||
          img->id().find("interface_") == 0)
        continue;

      auto position = m_content.get<Component::Position>(img->entity() + ":position");
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
  auto verb = m_content.get<Component::String> ("chosen_verb:text");
  std::string target_object = "";

  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(1.1 * m_verb_scale);

    const std::string& entity = m_collision->character_entity();
    if (entity != m_content.get<Component::String>("player:name")->value())
      if (auto name = m_content.request<Component::String>(entity + ":name"))
      {
        m_content.get<Component::Image>("verb_look:image")->set_scale(1.1 * m_verb_scale);
        target_object = name->value();
        auto state = m_content.request<Component::String>(m_collision->entity() + ":state");
        if (state && state->value() == "inventory")
        {
          if (verb->entity() == "verb_goto")
            verb = m_content.get<Component::String>("verb_look:text");
        }
      }
  }

  if (!m_content.request<Component::Action>("character:action")
      || verb != m_content.get<Component::String>("verb_goto:text"))
  {
    std::string text = verb->value() + " " + target_object;
    auto source = m_content.request<Component::String>("action:source");
    if (source)
    {
      if (auto name
          = m_content.request<Component::String>(source->value() + ":name"))
      {
        if (name->value() == target_object)
          target_object = "";
        text = verb->value() + " " + name->value() + " avec " + target_object;
      }
    }

    auto text_img
      = m_content.set<Component::Image>("chosen_verb:image",
                                        m_content.get<Component::Font>("interface:font"), "FFFFFF",
                                        text);

    text_img->set_relative_origin(0.5, 0.5);
    text_img->set_scale(0.8 * m_action_height / text_img->height());
  }
}

void Interface::update_inventory ()
{
  Status status = m_content.get<Component::Status>(GAME__STATUS)->value();
  if (status == IN_WINDOW || status == LOCKED)
  {
    m_content.get<Component::Image> ("interface_action:image")->z() = Config::inventory_over_depth;
    m_content.get<Component::Image> ("interface_verbs:image")->z() = Config::inventory_over_depth;
    m_content.get<Component::Image> ("interface_inventory:image")->z() = Config::inventory_over_depth;
    if (status == IN_WINDOW)
      m_content.get<Component::Image> ("window_overlay:image")->on() = true;
    return;
  }
  else
  {
    m_content.get<Component::Image> ("interface_action:image")->z() = Config::interface_depth;
    m_content.get<Component::Image> ("interface_verbs:image")->z() = Config::interface_depth;
    m_content.get<Component::Image> ("interface_inventory:image")->z() = Config::interface_depth;
    m_content.get<Component::Image> ("window_overlay:image")->on() = false;
  }

  auto background = m_content.get<Component::Image>("interface_inventory:image");

  auto inventory = m_content.get<Component::Inventory>("game:inventory");

  auto inv_pos = m_content.get<Component::Position>("interface_inventory:position");

  std::size_t position = inventory->position();
  for (std::size_t i = 0; i < inventory->size(); ++ i)
  {
    auto img = m_content.get<Component::Image>(inventory->get(i) + ":image");
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

      m_content.set<Component::Position>(inventory->get(i) + ":position", Point(x,y));


    }
    else
      img->on() = false;
  }

  bool left_on = (inventory->position() > 0);
  bool right_on =  (inventory->size() - inventory->position() > Config::displayed_inventory_size);

  if (m_layout == Config::WIDESCREEN)
  {
    m_content.get<Component::Image> ("inventory_arrow_0:image")->on() = left_on;
    m_content.get<Component::Image> ("inventory_arrow_background_0:image")->on() = left_on;
    m_content.get<Component::Image> ("inventory_arrow_1:image")->on() = false;
    m_content.get<Component::Image> ("inventory_arrow_background_1:image")->on() = false;
    m_content.get<Component::Image> ("inventory_arrow_2:image")->on() = right_on;
    m_content.get<Component::Image> ("inventory_arrow_background_2:image")->on() = right_on;
    m_content.get<Component::Image> ("inventory_arrow_3:image")->on() = false;
    m_content.get<Component::Image> ("inventory_arrow_background_3:image")->on() = false;
  }
  else
  {
    m_content.get<Component::Image> ("inventory_arrow_0:image")->on() = false;
    m_content.get<Component::Image> ("inventory_arrow_background_0:image")->on() = false;
    m_content.get<Component::Image> ("inventory_arrow_1:image")->on() = left_on;
    m_content.get<Component::Image> ("inventory_arrow_background_1:image")->on() = left_on;
    m_content.get<Component::Image> ("inventory_arrow_2:image")->on() = false;
    m_content.get<Component::Image> ("inventory_arrow_background_2:image")->on() = false;
    m_content.get<Component::Image> ("inventory_arrow_3:image")->on() = right_on;
    m_content.get<Component::Image> ("inventory_arrow_background_3:image")->on() = right_on;
  }
}


} // namespace Sosage::System
