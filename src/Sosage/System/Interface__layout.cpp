/*
  [src/Sosage/System/Interface__layout.cpp]
  Handles layout.

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
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Interface.h>

namespace Sosage::System
{

namespace C = Component;

void Interface::update_layout()
{
  int window_width = get<C::Int>("window:width")->value();
  int window_height = get<C::Int>("window:height")->value();

  int aspect_ratio = int(100. * window_width / double(window_height));

  Config::Layout layout = Config::Layout(get<C::Int>("interface:layout")->value());

  if (get<C::Status>(GAME__STATUS)->value() == CUTSCENE)
    layout = Config::STANDARD;

  if ((layout == Config::AUTO && aspect_ratio >= 200) ||
      (layout == Config::WIDESCREEN))
  {
    m_layout = Config::WIDESCREEN;
    layout_widescreen();
    vertical_layout();
  }
  else
  {
    if ((layout == Config::AUTO && aspect_ratio >= 115) ||
        (layout == Config::STANDARD))
    {
      m_layout = Config::STANDARD;
      layout_standard();
    }
    else if ((layout == Config::AUTO && aspect_ratio >= 75) ||
             (layout == Config::SQUARE))
    {
      m_layout = Config::SQUARE;
      layout_square();
    }
    else
    {
      m_layout = Config::PORTRAIT;
      layout_portrait();
    }

    horizontal_layout();
  }

  get<C::Image>("interface_action:image")->on() = false;
  get<C::Image>("interface_verbs:image")->on() = false;
  get<C::Image>("interface_inventory:image")->on() = false;

  bool blackscreen_on = get<C::Image>("blackscreen:image")->on();
  auto blackscreen = set<C::Image>("blackscreen:image",
                                   Config::world_width + get<C::Int>("interface:width")->value(),
                                   Config::world_height + get<C::Int>("interface:height")->value(),
                                   0, 0, 0, 255);
  blackscreen->on() = false;
  blackscreen->z() = Config::overlay_depth;
  blackscreen->collision() = UNCLICKABLE;

  remove ("dialog_choice_background:image", true);
  update_pause_screen();
}

void Interface::layout_widescreen()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 525;
  int interface_width = 180;

  set<C::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  set<C::Position> ("interface_action:position", Point(0, world_height));

  set<C::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_verbs:position", Point(world_width, 0));

  set<C::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_inventory:position",
                                      Point(world_width, interface_height));

  set<C::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  get<C::Image> ("turnicon:image")->on() = false;


  get<C::Int>("interface:width")->set(interface_width);
  get<C::Int>("interface:height")->set(m_action_height);
}

void Interface::layout_standard()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 150;
  int interface_width = world_width / 2;

  set<C::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  set<C::Position> ("interface_action:position", Point(0, world_height));

  set<C::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));

  set<C::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_inventory:position",
                                      Point(interface_width, m_action_height + world_height));

  set<C::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  get<C::Image> ("turnicon:image")->on() = false;

  get<C::Int>("interface:width")->set(0);
  get<C::Int>("interface:height")->set(interface_height + m_action_height);
}

void Interface::layout_square()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 50;
  int interface_height = 300;
  int interface_width = world_width;

  set<C::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  set<C::Position> ("interface_action:position", Point(0, world_height));

  set<C::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));

  set<C::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_inventory:position",
                                      Point(0, m_action_height + world_height + interface_height));

  set<C::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  get<C::Image> ("turnicon:image")->on() = false;

  get<C::Int>("interface:width")->set(0);
  get<C::Int>("interface:height")->set(2 * interface_height + m_action_height);
}

void Interface::layout_portrait()
{
  int world_width = 1920;
  int world_height = 1000;
  m_action_height = 100;
  int interface_height = 300;
  int interface_width = world_width;

  set<C::Image> ("interface_action:image", world_width, m_action_height, 0, 0, 0);
  set<C::Position> ("interface_action:position", Point(0, world_height));

  set<C::Image> ("interface_verbs:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_verbs:position", Point(0, m_action_height + world_height));

  set<C::Image> ("interface_inventory:image", interface_width, interface_height, 0, 0, 0);
  set<C::Position> ("interface_inventory:position",
                                      Point(0, m_action_height + world_height + interface_height));

  set<C::Position>("chosen_verb:position", Point(world_width / 2,
                                                                   world_height + m_action_height / 2));

  auto turnicon
    = get<C::Image> ("turnicon:image");

  set<C::Position>("turnicon:position",
                                     Point(0, world_height + 2 * interface_height + m_action_height));

  turnicon->on() = true;

  get<C::Int>("interface:width")->set(0);
  get<C::Int>("interface:height")->set(2 * interface_height + m_action_height
                                                         + turnicon->height());
}

void Interface::vertical_layout()
{
  auto interface_font = get<C::Font> ("interface:font");

  auto interface_verbs = get<C::Image>("interface_verbs:image");
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

  double min_scaling = (std::min)(h_scaling, w_scaling);

  int h_spacing = interface_height;
  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    int h = int(m_verbs[i]->height() * min_scaling);
    h_spacing -= h;
    m_verbs[i]->set_scale(min_scaling);
  }

  h_spacing /= 8;

  int x = Config::world_width + interface_width / 2;
  int current_y= h_spacing / 2;

  for (std::size_t i = 0; i < m_verbs.size(); ++ i)
  {
    auto img = m_verbs[i];

    int y = current_y + int(0.5 * img->height() * min_scaling);

    set<C::Position>(img->entity() + ":position", Point(x, y));

    current_y += img->height() * min_scaling + h_spacing;
  }

  set<C::Position>("chosen_verb:position", Point(Config::world_width / 2,
                                                                   Config::world_height + m_action_height / 2));

  m_verb_scale = min_scaling;

  get<C::Image> ("inventory_arrow_1:image")->on() = false;
  get<C::Image> ("inventory_arrow_background_1:image")->on() = false;
  get<C::Image> ("inventory_arrow_3:image")->on() = false;
  get<C::Image> ("inventory_arrow_background_3:image")->on() = false;

  auto inventory_position
    = get<C::Position> ("interface_inventory:position");
  auto inventory = get<C::Inventory>("game:inventory");
  bool left_on = (inventory->position() > 0);
  bool right_on =  (inventory->size() - inventory->position() > Config::displayed_inventory_size);

  auto arrow_img = get<C::Image> ("inventory_arrow_0:image");
  arrow_img->on() = left_on;
  arrow_img->set_scale (interface_width / double(arrow_img->width()));

  auto arrow_background_img
    = get<C::Image> ("inventory_arrow_background_0:image");
  arrow_background_img->on() = left_on;
  arrow_background_img->set_scale (interface_width / double(arrow_img->width()));

  set<C::Position> ("inventory_arrow_0:position",
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.05));
  set<C::Position> ("inventory_arrow_background_0:position",
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.05));


  arrow_img
    = get<C::Image> ("inventory_arrow_2:image");
  arrow_img->on() = right_on;
  arrow_img->set_scale (interface_width / double(arrow_img->width()));

  arrow_background_img
    = get<C::Image> ("inventory_arrow_background_2:image");
  arrow_background_img->on() = right_on;
  arrow_background_img->set_scale (interface_width / double(arrow_img->width()));

  set<C::Position> ("inventory_arrow_2:position",
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.95));
  set<C::Position> ("inventory_arrow_background_2:position",
                                      inventory_position->value()
                                      + Vector(interface_width / 2,
                                               interface_height * 0.95));
}

void Interface::horizontal_layout()
{
  auto interface_font = get<C::Font> ("interface:font");

  auto interface_verbs = get<C::Image>("interface_verbs:image");
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
  int current_ytop = Config::world_height + m_action_height + (h_spacing / 2);
  int current_ybottom = current_ytop + int(min_scaling * top_verbs_height) + h_spacing;

  for (std::size_t i = 0; i < m_verbs.size(); i += 2)
  {
    auto top = m_verbs[i];
    auto bottom = m_verbs[i+1];

    int xtop = current_xtop + int(0.5 * top->width() * min_scaling);
    int xbottom = current_xbottom + int(0.5 * bottom->width() * min_scaling);

    int ytop = current_ytop + int(0.5 * top->height() * min_scaling);
    int ybottom = current_ybottom + int(0.5 * bottom->height() * min_scaling);

    set<C::Position>(top->entity() + ":position", Point(xtop, ytop));
    set<C::Position>(bottom->entity() + ":position", Point(xbottom, ybottom));

    current_xtop += top->width() * min_scaling + w_spacing_top;
    current_xbottom += bottom->width() * min_scaling + w_spacing_bottom;
  }

  set<C::Position>("chosen_verb:position", Point(Config::world_width / 2,
                                                                   Config::world_height + m_action_height / 2));

  m_verb_scale = min_scaling;

  get<C::Image> ("inventory_arrow_0:image")->on() = false;
  get<C::Image> ("inventory_arrow_background_0:image")->on() = false;
  get<C::Image> ("inventory_arrow_2:image")->on() = false;
  get<C::Image> ("inventory_arrow_background_2:image")->on() = false;

  auto inventory_position
    = get<C::Position> ("interface_inventory:position");
  auto inventory = get<C::Inventory>("game:inventory");
  bool left_on = (inventory->position() > 0);
  bool right_on =  (inventory->size() - inventory->position() > Config::displayed_inventory_size);

  auto arrow_img
    = get<C::Image> ("inventory_arrow_1:image");
  arrow_img->on() = left_on;
  arrow_img->set_scale (interface_height / double(arrow_img->height()));

  auto arrow_background_img
    = get<C::Image> ("inventory_arrow_background_1:image");
  arrow_background_img->on() = left_on;
  arrow_background_img->set_scale (interface_height / double(arrow_img->height()));

  set<C::Position> ("inventory_arrow_1:position",
                                      inventory_position->value()
                                      + Vector(interface_width * 0.05,
                                               interface_height / 2));
  set<C::Position> ("inventory_arrow_background_1:position",
                                      inventory_position->value()
                                      + Vector(interface_width * 0.05,
                                               interface_height / 2));


  arrow_img
    = get<C::Image> ("inventory_arrow_3:image");
  arrow_img->on() = right_on;
  arrow_img->set_scale (interface_height / double(arrow_img->height()));

  arrow_background_img
    = get<C::Image> ("inventory_arrow_background_3:image");
  arrow_background_img->on() = right_on;
  arrow_background_img->set_scale (interface_height / double(arrow_img->height()));

  set<C::Position> ("inventory_arrow_3:position",
                                      inventory_position->value()
                                      + Vector(interface_width * 0.95,
                                               interface_height / 2));
  set<C::Position> ("inventory_arrow_background_3:position",
                                      inventory_position->value()
                                      + Vector(interface_width * 0.95,
                                               interface_height / 2));
}


} // namespace Sosage::System
