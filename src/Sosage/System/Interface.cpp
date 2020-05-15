/*
  [src/Sosage/System/Interface.cpp]
  Handles layout, interactions with buttons, collisions, etc.

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
#include <Sosage/Component/Console.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/System/Interface.h>

namespace Sosage::System
{

Interface::Interface (Content& content)
  : m_content (content)
  , m_auto_layout (true)
  , m_layout (INIT)
  , m_action_min_height(50)
  , m_interface_min_height(150)
{

}

void Interface::run()
{
  if (m_content.request<Component::Event>("window:set_auto"))
  {
    m_auto_layout = true;
    m_content.remove("window:set_auto");
    m_content.set<Component::Event>("window:rescaled");
  }
  else if (m_content.request<Component::Event>("window:set_widescreen"))
  {
    m_auto_layout = false;
    m_layout = WIDESCREEN;
    m_content.remove("window:set_widescreen");
    m_content.set<Component::Event>("window:rescaled");
  }
  else if (m_content.request<Component::Event>("window:set_standard"))
  {
    m_auto_layout = false;
    m_layout = STANDARD;
    m_content.remove("window:set_standard");
    m_content.set<Component::Event>("window:rescaled");
  }
  else if (m_content.request<Component::Event>("window:set_square"))
  {
    m_auto_layout = false;
    m_layout = SQUARE;
    m_content.remove("window:set_square");
    m_content.set<Component::Event>("window:rescaled");
  }
  else if (m_content.request<Component::Event>("window:set_portrait"))
  {
    m_auto_layout = false;
    m_layout = PORTRAIT;
    m_content.remove("window:set_portrait");
    m_content.set<Component::Event>("window:rescaled");
  }
  
  if (m_content.request<Component::Event>("window:rescaled"))
    update_responsive();
  
  auto cursor = m_content.request<Component::Position>("cursor:position");
  if (cursor)
    detect_collision (cursor);

  auto clicked
    = m_content.request<Component::Event>("cursor:clicked");
  if (clicked && m_collision)
  {
    std::string status = m_content.get<Component::State>("game:status")->value();
    if (status == "window")
      window_clicked();
    else if (status == "code")
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
          = m_content.get<Component::Text> ("chosen_verb:text")->entity();
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
  std::string color_str = m_content.get<Component::Text> ("interface:color")->value();

  for (const auto& verb : { std::make_pair (std::string("open"), std::string("Ouvrir")),
        std::make_pair (std::string("close"), std::string("Fermer")),
        std::make_pair (std::string("give"), std::string("Donner")),
        std::make_pair (std::string("take"), std::string("Prendre")),
        std::make_pair (std::string("look"), std::string("Regarder")),
        std::make_pair (std::string("talk"), std::string("Parler")),
        std::make_pair (std::string("use"), std::string("Utiliser")),
        std::make_pair (std::string("move"), std::string("Déplacer")) })
  {
    m_content.set<Component::Text> ("verb_" + verb.first + ":text", verb.second);
    auto verb_img
      = m_content.set<Component::Image> ("verb_" + verb.first + ":image",
                                         interface_font, color_str, verb.second);
    m_content.set<Component::Position> ("verb_" + verb.first + ":position",
                                        Point(0,0));
    m_verbs.push_back (verb_img);
    verb_img->set_relative_origin(0.5, 0.5);
  }

  auto verb_goto = m_content.set<Component::Text> ("verb_goto:text", "Aller vers");
  m_content.set<Component::Variable>("chosen_verb:text", verb_goto);

  // Create pause screen
  m_content.set<Component::Boolean>("game:paused", false);
  
  // Create status variable
  m_content.set<Component::State>("game:status", "idle");

  auto pause_screen_pos
    = m_content.set<Component::Position>("pause_screen:position", Point(0, 0));
  m_content.set<Component::Variable>("window_overlay:position", pause_screen_pos);

  update_responsive();
}

void Interface::update_responsive()
{
  int world_width = Sosage::world_width;
  int world_height = Sosage::world_height;
  int window_width = config().window_width;
  int window_height = config().window_height;

  int aspect_ratio = int(100. * window_width / double(window_height));

  if ((m_auto_layout && aspect_ratio >= 200) ||
      (!m_auto_layout && m_layout == WIDESCREEN))
  {
    if (m_auto_layout)
      m_layout = WIDESCREEN;
    
    interface_widescreen();
    vertical_layout();
  }
  else
  {
    if ((m_auto_layout && aspect_ratio >= 115) ||
        (!m_auto_layout && m_layout == STANDARD))
    {
      if (m_auto_layout)
        m_layout = STANDARD;
      interface_standard();
    }
    else if ((m_auto_layout && aspect_ratio >= 75) ||
             (!m_auto_layout && m_layout == SQUARE))
    {
      if (m_auto_layout)
        m_layout = SQUARE;
      interface_square();
    }
    else
    {
      if (m_auto_layout)
        m_layout = PORTRAIT;
      interface_portrait();
    }
    
    horizontal_layout();
  }

  update_pause_screen();
}

void Interface::interface_widescreen()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 525;
  int interface_width = 180;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(world_width, 0));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(world_width, interface_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  m_content.get<Component::Image> ("turnicon:image")->on() = false;
  
  
  config().interface_width = interface_width;
  config().interface_height = m_action_height;
}

void Interface::interface_standard()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 150;
  int interface_width = world_width / 2;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(interface_width, m_action_height + world_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  m_content.get<Component::Image> ("turnicon:image")->on() = false;

  config().interface_width = 0;
  config().interface_height = interface_height + m_action_height;
}

void Interface::interface_square()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 300;
  int interface_width = world_width;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(0, m_action_height + world_height + interface_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  m_content.get<Component::Image> ("turnicon:image")->on() = false;

  config().interface_width = 0;
  config().interface_height = 2 * interface_height + m_action_height;
}

void Interface::interface_portrait()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 100;
  int interface_height = 300;
  int interface_width = world_width;

  m_content.set<Component::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_action:position", Point(0, world_height));
  
  m_content.set<Component::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));
                                                                        
  m_content.set<Component::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  m_content.set<Component::Position> ("interface_inventory:position",
                                      Point(0, m_action_height + world_height + interface_height));
  
  m_content.set<Component::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  auto turnicon
    = m_content.get<Component::Image> ("turnicon:image");

  m_content.set<Component::Position>("turnicon:position",
                                     Point(0, world_height + 2 * interface_height + m_action_height));

  turnicon->on() = true;

  config().interface_width = 0;
  config().interface_height = 2 * interface_height + m_action_height + turnicon->height();
}

void Interface::vertical_layout()
{
  auto interface_font = m_content.get<Component::Font> ("interface:font");
  
  int top_width = 0;
  auto interface_verbs = m_content.get<Component::Image>("interface_verbs:image");
  int interface_width = interface_verbs->width();
  int interface_height = interface_verbs->height();

  int min_w_spacing = 30;
  int min_h_spacing = 15;
  
  int verbs_width = 0;
  int verbs_height = 0;

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
    m_verbs[i]->set_scale(1);

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int w = m_verbs[i]->width();
    int h = m_verbs[i]->height() + min_h_spacing;
    verbs_width = (std::max)(verbs_width, w);
    verbs_height += h;
  }

  int min_verbs_width = verbs_width + min_w_spacing * 2;
  int min_verbs_height = verbs_height + min_h_spacing * 2;

  double w_scaling = interface_width / double(min_verbs_width);
  double h_scaling = interface_height / double(min_verbs_height);

  DBG_CERR << "Scaling = " << w_scaling << "*" << h_scaling << std::endl;

  double min_scaling = (std::min)(h_scaling, w_scaling);

  int h_spacing = interface_height;
  DBG_CERR << h_spacing << " ";
  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int h = int(m_verbs[i]->height() * min_scaling);
    h_spacing -= h;
    m_verbs[i]->set_scale(min_scaling);
  }

  h_spacing /= 8;
  DBG_CERR << h_spacing << " ";

  int x = Sosage::world_width + interface_width / 2;
  int current_y= h_spacing / 2;

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    auto img = m_verbs[i];

    int y = current_y + 0.5 * img->height() * min_scaling;

    m_content.set<Component::Position>(img->entity() + ":position", Point(x, y));

    current_y += img->height() * min_scaling + h_spacing;
  }

  m_content.set<Component::Position>("chosen_verb:position", Point(Sosage::world_width / 2,
                                                                   Sosage::world_height + m_action_height / 2));

  m_verb_scale = min_scaling;

  m_content.get<Component::Image> ("inventory_arrow_1:image")->on() = false;
  m_content.get<Component::Image> ("inventory_arrow_background_1:image")->on() = false;
  m_content.get<Component::Image> ("inventory_arrow_3:image")->on() = false;
  m_content.get<Component::Image> ("inventory_arrow_background_3:image")->on() = false;

  auto inventory_position
    = m_content.get<Component::Position> ("interface_inventory:position");
  auto inventory = m_content.get<Component::Inventory>("game:inventory");
  bool left_on = (inventory->position() > 0);
  bool right_on =  (inventory->size() - inventory->position() > Sosage::displayed_inventory_size);
  
  auto arrow_img = m_content.get<Component::Image> ("inventory_arrow_0:image");
  arrow_img->on() = left_on;
  arrow_img->set_scale (interface_width / double(arrow_img->width()));
  
  auto arrow_background_img 
    = m_content.get<Component::Image> ("inventory_arrow_background_0:image");
  arrow_background_img->on() = left_on;
  arrow_background_img->set_scale (interface_width / double(arrow_img->width()));
  
  m_content.set<Component::Position> ("inventory_arrow_0:position",
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.05));
  m_content.set<Component::Position> ("inventory_arrow_background_0:position",
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.05));
  

  arrow_img 
    = m_content.get<Component::Image> ("inventory_arrow_2:image");
  arrow_img->on() = right_on;
  arrow_img->set_scale (interface_width / double(arrow_img->width()));

  arrow_background_img
    = m_content.get<Component::Image> ("inventory_arrow_background_2:image");
  arrow_background_img->on() = right_on;
  arrow_background_img->set_scale (interface_width / double(arrow_img->width()));
  
  m_content.set<Component::Position> ("inventory_arrow_2:position", 
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.95));
  m_content.set<Component::Position> ("inventory_arrow_background_2:position",
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.95));
}

void Interface::horizontal_layout()
{
  auto interface_font = m_content.get<Component::Font> ("interface:font");
  
  int top_width = 0;
  auto interface_verbs = m_content.get<Component::Image>("interface_verbs:image");
  int interface_width = interface_verbs->width();
  int interface_height = interface_verbs->height();

  int min_w_spacing = 40;
  int min_h_spacing = 15;
  
  int top_verbs_width = 0;
  int bottom_verbs_width = 0;
  int top_verbs_height = 0;
  int bottom_verbs_height = 0;
  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
    m_verbs[i]->set_scale(1);

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int w = m_verbs[i]->width() + min_w_spacing;
    int h = m_verbs[i]->height();
    if (i % 2 == 0)
    {
      top_verbs_width += w;
      top_verbs_height = (std::max)(top_verbs_height, h);
    }
    else
    {
      bottom_verbs_width += w;
      bottom_verbs_height = (std::max)(bottom_verbs_height, h);
    }
  }

  int min_verbs_width = (std::max)(top_verbs_width, bottom_verbs_width);
  int min_verbs_height = top_verbs_height + bottom_verbs_height + min_h_spacing * 2;

  double w_scaling = interface_width / double(min_verbs_width);
  double h_scaling = interface_height / double(min_verbs_height);

  DBG_CERR << "Scaling = " << w_scaling << "*" << h_scaling << std::endl;

  double min_scaling = (std::min)(h_scaling, w_scaling);

  int w_spacing_top = interface_width;
  int w_spacing_bottom = interface_width;
  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int w = int(m_verbs[i]->width() * min_scaling);
    if (i % 2 == 0)
      w_spacing_top -= w;
    else
      w_spacing_bottom -= w;
    m_verbs[i]->set_scale(min_scaling);
  }

  int h_spacing = interface_height - int(min_scaling * (top_verbs_height + top_verbs_height));

  w_spacing_top /= 4;
  w_spacing_bottom /= 4;
  h_spacing /= 2;

  int current_xtop = w_spacing_top / 2;
  int current_xbottom = w_spacing_bottom / 2;
  int current_ytop = Sosage::world_height + m_action_height + (h_spacing / 2);
  int current_ybottom = current_ytop + int(min_scaling * top_verbs_height) + h_spacing;

  for (std::size_t i = 0; i < m_verbs.size(); i += 2)
  {
    auto top = m_verbs[i];
    auto bottom = m_verbs[i+1];

    int xtop = current_xtop + 0.5 * top->width() * min_scaling;
    int xbottom = current_xbottom + 0.5 * bottom->width() * min_scaling;
    
    int ytop = current_ytop + 0.5 * top->height() * min_scaling;
    int ybottom = current_ybottom + 0.5 * bottom->height() * min_scaling;

    m_content.set<Component::Position>(top->entity() + ":position", Point(xtop, ytop));
    m_content.set<Component::Position>(bottom->entity() + ":position", Point(xbottom, ybottom));

    current_xtop += top->width() * min_scaling + w_spacing_top;
    current_xbottom += bottom->width() * min_scaling + w_spacing_bottom;
  }

  m_content.set<Component::Position>("chosen_verb:position", Point(Sosage::world_width / 2,
                                                                   Sosage::world_height + m_action_height / 2));

  m_verb_scale = min_scaling;
  
  m_content.get<Component::Image> ("inventory_arrow_0:image")->on() = false;
  m_content.get<Component::Image> ("inventory_arrow_background_0:image")->on() = false;
  m_content.get<Component::Image> ("inventory_arrow_2:image")->on() = false;
  m_content.get<Component::Image> ("inventory_arrow_background_2:image")->on() = false;

  auto inventory_position
    = m_content.get<Component::Position> ("interface_inventory:position");
  auto inventory = m_content.get<Component::Inventory>("game:inventory");
  bool left_on = (inventory->position() > 0);
  bool right_on =  (inventory->size() - inventory->position() > Sosage::displayed_inventory_size);
  
  auto arrow_img 
    = m_content.get<Component::Image> ("inventory_arrow_1:image");
  arrow_img->on() = left_on;
  arrow_img->set_scale (interface_height / double(arrow_img->height()));
  
  auto arrow_background_img 
    = m_content.get<Component::Image> ("inventory_arrow_background_1:image");
  arrow_background_img->on() = left_on;
  arrow_background_img->set_scale (interface_height / double(arrow_img->height()));
  
  m_content.set<Component::Position> ("inventory_arrow_1:position",
                                      inventory_position->value()
                                      + Vector(interface_width * 0.05,
                                               interface_height / 2));
  m_content.set<Component::Position> ("inventory_arrow_background_1:position",
                                      inventory_position->value()
                                      + Vector(interface_width * 0.05,
                                               interface_height / 2));
  

  arrow_img 
    = m_content.get<Component::Image> ("inventory_arrow_3:image");
  arrow_img->on() = right_on;
  arrow_img->set_scale (interface_height / double(arrow_img->height()));

  arrow_background_img
    = m_content.get<Component::Image> ("inventory_arrow_background_3:image");
  arrow_background_img->on() = right_on;
  arrow_background_img->set_scale (interface_height / double(arrow_img->height()));
  
  m_content.set<Component::Position> ("inventory_arrow_3:position", 
                                      inventory_position->value()
                                      + Vector(interface_width * 0.95,
                                               interface_height / 2));
  m_content.set<Component::Position> ("inventory_arrow_background_3:position",
                                      inventory_position->value()
                                      + Vector(interface_width * 0.95,
                                               interface_height / 2));
}

void Interface::window_clicked()
{
  auto window = m_content.get<Component::Image>("game:window");
  if (m_collision != window)
  {
    window->on() = false;
    m_content.get<Component::State>("game:status")->set ("idle");
  }
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
    m_content.get<Component::State>("game:status")->set ("idle");
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
    ->set(m_content.get<Component::Text>(m_collision->entity() + ":text"));
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
  
  if (m_content.request<Component::Text>(m_collision->entity() + ":name"))
  {
    auto action
      = m_content.request<Component::Action> (m_collision->entity() + ":" + verb);
    if (action)
    {
      m_content.set<Component::Variable>("character:action", action);
      action_found = true;
      m_content.remove ("action:source", true);
    }
    else
    {
      if (verb == "use")
      {
        if (auto source = m_content.request<Component::Text>("action:source"))
        {
          if (source->value() == m_collision->entity())
          {
            m_content.remove("cursor:clicked");
            return;
          }

          auto action
            = m_content.request<Component::Action> (m_collision->entity() + ":use_"
                                                    + source->value());
          if (action)
          {
            m_content.set<Component::Variable>("character:action", action);
            action_found = true;
          }
        }
        else
        {
          auto state
            = m_content.request<Component::State>(m_collision->entity() + ":state");
          if (state && (state->value() == "inventory"))
          {
            m_content.set<Component::Text>("action:source", m_collision->entity());
            m_content.remove("cursor:clicked");
            return;
          }
        }
      }

      m_content.remove ("action:source", true);
    }

    if (!action_found)
    {
      if (verb == "goto")
      {
        m_content.set<Component::Variable>("cursor:target", m_collision);
        return;
      }
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
      auto state = m_content.request<Component::State>(m_collision->entity() + ":state");
      if (state && (state->value() == "inventory"))
      {
        auto action
          = m_content.request<Component::Action> (m_collision->entity() + ":look");
        if (action)
          m_content.set<Component::Variable>("character:action", action);
        m_content.remove("cursor:clicked");
      }
      else
      {
        m_content.set<Component::Variable>("cursor:target", m_collision);
        return;
      }
    }
  }

  m_content.get<Component::Variable> ("chosen_verb:text")
    ->set(m_content.get<Component::Text>("verb_goto:text"));
  
  m_content.remove("cursor:clicked");
}


void Interface::update_pause_screen()
{
  auto interface_font = m_content.get<Component::Font> ("interface:font");

  auto pause_screen_img
    = Component::make_handle<Component::Image>
    ("pause_screen:image",
     Sosage::world_width + config().interface_width,
     Sosage::world_height + config().interface_height,
     0, 0, 0, 192);
  pause_screen_img->z() += 10;
      
  // Create pause screen
  auto paused
    = m_content.get<Component::Boolean>("game:paused");
  
  auto pause_screen
    = m_content.set<Component::Conditional>("pause_screen:conditional", paused,
                                            pause_screen_img, Component::Handle());

  auto pause_text_img
    = Component::make_handle<Component::Image>("pause_text:image", interface_font, "FFFFFF", "PAUSE");
  pause_text_img->z() += 10;
  pause_text_img->set_relative_origin(0.5, 0.5);

  auto window_overlay_img
    = m_content.set<Component::Image>("window_overlay:image",
                                      Sosage::world_width + config().interface_width,
                                      Sosage::world_height + config().interface_height,
                                      0, 0, 0, 128);
  window_overlay_img->z() = Sosage::interface_depth;
  window_overlay_img->on() = false;

  auto pause_text
    = m_content.set<Component::Conditional>("pause_text:conditional", paused,
                                            pause_text_img, Component::Handle());
    
  m_content.set<Component::Position>("pause_text:position", Point(Sosage::world_width / 2,
                                                                  Sosage::world_height / 2));

}

void Interface::detect_collision (Component::Position_handle cursor)
{
  // Deactive previous collisions
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(m_verb_scale);
  }

  m_collision = Component::Image_handle();
  
  for (const auto& e : m_content)
    if (auto img = Component::cast<Component::Image>(e))
    {
      if (!img->on() ||
          img->id().find("character") == 0 ||
          img->id().find("debug") == 0 ||
          img->id().find("chosen_verb") == 0 ||
          img->id().find("interface_") == 0)
        continue;
      
      auto p = m_content.get<Component::Position>(img->entity() + ":position");

      Point screen_position = p->value() - img->core().scaling * Vector(img->origin());
      int xmin = screen_position.x();
      int ymin = screen_position.y();
      int xmax = xmin + (img->core().scaling * (img->xmax() - img->xmin()));
      int ymax = ymin + (img->core().scaling * (img->ymax() - img->ymin()));

      if (cursor->value().x() < xmin ||
          cursor->value().x() > xmax ||
          cursor->value().y() < ymin ||
          cursor->value().y() > ymax)
        continue;

      if (!img->box_collision())
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
  auto verb = m_content.get<Component::Text> ("chosen_verb:text");
  std::string target_object = "";
  
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(1.1 * m_verb_scale);
    if (auto name = m_content.request<Component::Text>(m_collision->entity() + ":name"))
    {
      target_object = name->value();
      auto state = m_content.request<Component::State>(m_collision->entity() + ":state");
      if (state && state->value() == "inventory")
      {
        if (verb->entity() == "verb_goto")
          verb = m_content.get<Component::Text>("verb_look:text");
      }
    }
  }

  if (!m_content.request<Component::Action>("character:action")
      || verb != m_content.get<Component::Text>("verb_goto:text"))
  {
    std::string text = verb->value() + " " + target_object;
    auto source = m_content.request<Component::Text>("action:source");
    if (source)
    {
      if (auto name
          = m_content.request<Component::Text>(source->value() + ":name"))
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
  if (m_content.get<Component::State>("game:status")->value() == "window"
      || m_content.get<Component::State>("game:status")->value() == "locked")
  {
    m_content.get<Component::Image> ("interface_action:image")->z() = Sosage::inventory_over_depth;
    m_content.get<Component::Image> ("interface_verbs:image")->z() = Sosage::inventory_over_depth;
    m_content.get<Component::Image> ("interface_inventory:image")->z() = Sosage::inventory_over_depth;
    if (m_content.get<Component::State>("game:status")->value() == "window")
      m_content.get<Component::Image> ("window_overlay:image")->on() = true;
    return;
  }
  else
  {
    m_content.get<Component::Image> ("interface_action:image")->z() = Sosage::interface_depth;
    m_content.get<Component::Image> ("interface_verbs:image")->z() = Sosage::interface_depth;
    m_content.get<Component::Image> ("interface_inventory:image")->z() = Sosage::interface_depth;
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
    
    if (position <= i && i < position + Sosage::displayed_inventory_size)
    {
      std::size_t pos = i - position;
      double relative_pos = (1 + pos) / double(Sosage::displayed_inventory_size + 1);
      img->on() = true;

      int x, y;

      if (m_layout == WIDESCREEN)
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
  bool right_on =  (inventory->size() - inventory->position() > Sosage::displayed_inventory_size);

  if (m_layout == WIDESCREEN)
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
