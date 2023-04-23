/*
  [src/Sosage/System/Menu__creations.cpp]
  Functions for creating menus.

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
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/Component/Menu.h>
#include <Sosage/System/Menu.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/datetime.h>

#include <queue>

namespace Sosage::System
{

namespace C = Component;

void Menu::init_loadsave_menus ()
{
  std::deque<C::Tuple_handle<std::string, double, int>>
      save_infos;

  for (std::size_t idx = 0; idx < 5; ++ idx)
    if (auto info = request<C::Tuple<std::string, double, int>>
        ("Save_" + std::to_string(idx+1), "info"))
      save_infos.emplace_back (info);
    else
      break;

  std::size_t nb_saves = save_infos.size();
  // Additional free slot if enough space
  if (nb_saves < 5)
    nb_saves ++;

  auto save_menu = set<C::Menu>("Save", "menu");
  save_menu->split(VERTICALLY, nb_saves + 2);
  make_text_menu_title((*save_menu)[0], "Save");

  int y = Config::settings_menu_start;
  for (std::size_t idx = 1; idx < nb_saves + 1; ++ idx)
  {
    auto node = (*save_menu)[idx];
    std::string id = "Save_" + std::to_string(idx);
    C::Image_handle button;
    C::Position_handle pos_button;
    std::tie (button, pos_button) = make_settings_button (id, y);
    node.init(button, pos_button);

    set<C::String>(id, "effect", id);

    node.split(VERTICALLY, 2);

    std::string slot = std::to_string(idx) + ". ";
    std::string info = locale_get("Free_slot", "text");

    if (idx - 1 < save_infos.size())
    {
      auto saveinfo = save_infos[idx - 1];
      slot += locale(saveinfo->get<0>());
      info = date_to_string(saveinfo->get<2>());
      info += " (" + locale_get("Game_time", "text")
              + " = " + time_to_string(saveinfo->get<1>()) + ")";
    }

    // Slot
    {
      C::Image_handle img;
      C::Position_handle pos;
      std::tie (img, pos) = make_settings_title (id, "slot", y, slot);
      node[0].init(img, pos);
    }

    // Number value
    {
      C::Image_handle img;
      C::Position_handle pos;
      std::tie (img, pos) = make_settings_subtitle (id, "info", y, info);
      node[1].init(img, pos);
    }

    y += Config::settings_menu_height + Config::settings_menu_margin;
  }

  make_oknotok_item ((*save_menu)[nb_saves + 1], true);

  if (auto info = request<C::Tuple<std::string, double, int>>("Save_auto", "info"))
    save_infos.emplace_front (info);

  auto load_menu = set<C::Menu>("Load", "menu");
  std::size_t load_size = save_infos.size();
  if (load_size == 0)
    load_size = 1;
  load_menu->split(VERTICALLY, load_size + 2);
  make_text_menu_title((*load_menu)[0], "Load");

  y = Config::settings_menu_start;
  for (std::size_t idx = 1; idx < save_infos.size() + 1; ++ idx)
  {
    auto node = (*load_menu)[idx];
    auto saveinfo = save_infos[idx - 1];
    std::string save_id (saveinfo->entity().begin() + saveinfo->entity().find('_') + 1,
                         saveinfo->entity().end());
    std::string id = "Load_" + save_id;
    C::Image_handle button;
    C::Position_handle pos_button;
    std::tie (button, pos_button) = make_settings_button (id, y);
    node.init(button, pos_button);

    set<C::String>(id, "effect", id);

    node.split(VERTICALLY, 2);

    std::string slot = save_id + ". ";
    if (save_id == "auto")
      slot = "(Auto) ";
    slot += locale(saveinfo->get<0>());
    std::string info = date_to_string(saveinfo->get<2>());
    info += " (" + locale_get("Game_time", "text")
            + " = " + time_to_string(saveinfo->get<1>()) + ")";

    // Slot
    {
      C::Image_handle img;
      C::Position_handle pos;
      std::tie (img, pos) = make_settings_title (id, "slot", y, slot);
      node[0].init(img, pos);
    }

    // Number value
    {
      C::Image_handle img;
      C::Position_handle pos;
      std::tie (img, pos) = make_settings_subtitle (id, "info", y, info);
      node[1].init(img, pos);
    }

    y += Config::settings_menu_height + Config::settings_menu_margin;
  }

  if (save_infos.size() == 0)
    make_text_menu_text((*load_menu)[1], "No_savegame");

  make_oknotok_item ((*load_menu)[load_size + 1], true);
}



void Menu::make_exit_menu_item (Component::Menu::Node node, const std::string& id, int y)
{
  auto reference = get<C::Position>("Menu", "reference");

  if (id == "Menu_logo")
  {
    auto img = get<C::Image>(id , "image");
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_relative_origin(0.5, 0.5);
    auto pos = set<C::Relative_position>(id , "position", reference, Vector(240, y));
    node.init(img, pos);
    return;
  }

  auto font = get<C::Font>("Interface", "font");
  auto text = request<C::String>(id , "text");

  node.split(BUTTON, 1);
  node[0].split(HORIZONTALLY, 2);

  // might be reused
  auto icon = get<C::Image>(id + "_icon", "image");
  auto img = request<C::Image>("Exit_" + id , "image");
  C::Position_handle pos, pos_icon;
  if (!img)
  {
    img = set<C::Image>("Exit_" + id , "image", font, "FFFFFF", locale(text->value()));
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(0.5);
    img->set_relative_origin(0, 0.5);
    img->set_collision(UNCLICKABLE);

    pos = set<C::Relative_position>("Exit_" + id , "position", reference, Vector (Config::exit_menu_text, y));
    pos_icon = set<C::Relative_position>(id + "_icon", "position", reference, Vector (Config::menu_margin, y));
  }
  else
  {
    pos = get<C::Position>("Exit_" + id , "position");
    icon = get<C::Image>(id + "_icon", "image");
    pos_icon = get<C::Position>(id + "_icon", "position");
  }
  node[0][0].init(icon, pos_icon);
  node[0][1].init(img, pos);

  // Create button
  auto button = request<C::Image>(id + "_button", "image");
  C::Position_handle pos_button;
  if (!button)
  {
    button = set<C::Image>(id + "_button", "image", get<C::Image>("Menu_main_button", "image"));
    //button = set<C::Image>(id + "_button", "image", Config::exit_menu_button_width, Config::exit_menu_button_height, 0, 0, 0, 64);
    button->z() = Config::menu_button_depth;
    button->set_relative_origin(0.5, 0.5);
    button->on() = false;
    pos_button = set<C::Relative_position>(id + "_button", "position", reference, Vector (240, y));
  }
  else
    pos_button = get<C::Position>(id + "_button", "position");
  node.init(button, pos_button);

  set<C::String>(id , "effect", id);
}

void Menu::make_oknotok_item (Component::Menu::Node node, bool only_ok)
{
  C::Image_handle ok, cancel, ok_alone, ok_button, cancel_button, ok_alone_button;
  C::Position_handle ok_pos, cancel_pos, ok_alone_pos,
      ok_button_pos, cancel_button_pos, ok_alone_button_pos;

  ok_alone = request<C::Image>("Ok_alone_icon", "image");

  // Only create once
  if (!ok_alone)
  {
    auto reference = get<C::Position>("Menu", "reference");

    ok = get<C::Image>("Ok_icon", "image");
    ok->z() = Config::menu_text_depth;
    ok->on() = false;
    ok->set_relative_origin(0.5, 0.5);
    ok->set_collision(UNCLICKABLE);
    ok_alone = set<C::Image>("Ok_alone_icon", "image", ok);

    ok_pos = set<C::Relative_position>("Ok_icon", "position", reference,
                                       Vector (Config::menu_ok_x, Config::menu_oknotok_y));
    ok_alone_pos = set<C::Relative_position>("Ok_alone_icon", "position", reference,
                                             Vector (240, Config::menu_oknotok_y));

    ok_button = set<C::Image>("Ok_button", "image", 230, 50, 0, 0, 0, 64);
    ok_button->z() = Config::menu_button_depth;
    ok_button->set_relative_origin(0.5, 0.5);
    ok_button->on() = false;
    ok_alone_button = set<C::Image>("Ok_alone_button", "image", 460, 50, 0, 0, 0, 64);
    ok_alone_button->z() = Config::menu_button_depth;
    ok_alone_button->set_relative_origin(0.5, 0.5);
    ok_alone_button->on() = false;
    ok_button_pos = set<C::Relative_position>("Ok_button", "position", ok_pos, Vector(0,0));
    ok_alone_button_pos = set<C::Relative_position>("Ok_alone_button", "position", ok_alone_pos, Vector(0,0));

    set<C::String>("Ok", "effect", "Ok");
    set<C::String>("Ok_alone", "effect", "Ok");

    cancel = get<C::Image>("Cancel_icon", "image");
    cancel->z() = Config::menu_text_depth;
    cancel->on() = false;
    cancel->set_relative_origin(0.5, 0.5);
    cancel->set_collision(UNCLICKABLE);
    cancel_pos = set<C::Relative_position>("Cancel_icon", "position", reference,
                                           Vector (Config::menu_notok_x, Config::menu_oknotok_y));

    cancel_button = set<C::Image>("Cancel_button", "image", 230, 50, 0, 0, 0, 64);
    cancel_button->set_relative_origin(0.5, 0.5);
    cancel_button->on() = false;
    cancel_button_pos = set<C::Relative_position>("Cancel_button", "position", cancel_pos, Vector(0,0));
    cancel_button->z() = Config::menu_button_depth;

    set<C::String>("Cancel", "effect", "Cancel");
  }
  else
  {
    ok = get<C::Image>("Ok_icon", "image");
    cancel = get<C::Image>("Cancel_icon", "image");
    ok_button = get<C::Image>("Ok_button", "image");
    cancel_button = get<C::Image>("Cancel_button", "image");
    ok_alone_button = get<C::Image>("Ok_alone_button", "image");
    ok_pos = get<C::Position>("Ok_icon", "position");
    cancel_pos = get<C::Position>("Cancel_icon", "position");
    ok_alone_pos = get<C::Position>("Ok_alone_icon", "position");
    ok_button_pos = get<C::Position>("Ok_button", "position");
    cancel_button_pos = get<C::Position>("Cancel_button", "position");
    ok_alone_button_pos = get<C::Position>("Ok_alone_button", "position");
  }

  if (only_ok)
  {
    node.split(BUTTON, 1);
    node.init(ok_alone_button, ok_alone_button_pos);
    node[0].init(ok_alone, ok_alone_pos);
  }
  else
  {
    node.split(HORIZONTALLY, 2);
    node[0].split(BUTTON, 1);
    node[0].init(ok_button, ok_button_pos);
    node[0][0].init(ok, ok_pos);
    node[1].split(BUTTON, 1);
    node[1].init(cancel_button, cancel_button_pos);
    node[1][0].init(cancel, cancel_pos);
  }
}


void Menu::make_text_menu_title (Component::Menu::Node node, const std::string& id)
{
  auto reference = get<C::Position>("Menu", "reference");
  auto font = get<C::Font>("Interface", "font");
  auto text = get<C::String>(id , "text");
  auto img = set<C::Image>("Title_" + id , "image", font, "FFFFFF", locale(text->value()));
  img->z() = Config::menu_text_depth;
  img->on() = false;
  img->set_scale(0.75);
  img->set_relative_origin(0.5, 0.5);
  img->set_collision(UNCLICKABLE);
  auto pos = set<C::Relative_position>("Title_" + id , "position", reference,
                                       Point(240, Config::menu_margin));
  node.init(img, pos);
}

void Menu::make_text_menu_text (Component::Menu::Node node, const std::string& id, bool credits)
{
  auto reference = get<C::Position>("Menu", "reference");
  auto font = get<C::Font>("Interface", "light_font");
  auto text = get<C::String>(id , "text");

  const std::string& str = locale(text->value());
  std::size_t pos = str.find('\n');
  if (pos == std::string::npos)
    pos = str.size();
  std::vector<std::string> lines;
  std::size_t begin = 0;
  do
  {
    lines.emplace_back (str.begin() + begin, str.begin() + pos);
    if (lines.back() == "")
      lines.back() = " ";
    begin = pos + 1;
    pos = str.find('\n', pos+1);
  }
  while (pos != std::string::npos);

  double scale = (credits ? 0.4 : 0.5);
  node.split(VERTICALLY, lines.size());
  int y = (credits ? Config::settings_menu_start : Config::exit_menu_start);

  for (std::size_t i = 0; i < lines.size(); ++ i)
  {
    auto img = set<C::Image>(text->entity() + "_" + std::to_string(i)
                             , "image", font, "FFFFFF", lines[i]);
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(scale);
    img->set_relative_origin(0, 0);
    img->set_collision(UNCLICKABLE);
    auto pos = set<C::Relative_position>(text->entity() + "_" + std::to_string(i)
                                         , "position", reference,
                                         Point(Config::menu_small_margin * 2. * scale, y));
    node[i].init(img, pos);
    y += Config::menu_small_margin * 2. * scale;
  }
}

void Menu::make_settings_item (Component::Menu::Node node, const std::string& id, int y)
{
  auto reference = get<C::Position>("Menu", "reference");
  auto light_font = get<C::Font>("Interface", "light_font");

  // Create button

  C::Image_handle button;
  C::Position_handle pos_button;
  std::tie (button, pos_button) = make_settings_button (id, y);
  node.init(button, pos_button);

  set<C::String>(id, "effect", id);

  node.split(HORIZONTALLY, 4);

  // Setting title
  {
    C::Image_handle img;
    C::Position_handle pos;
    std::tie (img, pos) = make_settings_title (id, "setting", y,
                                               locale_get(id, "text"));
    node[0].init(img, pos);
  }

  // Setting value
  {
    std::vector<std::string> possible_values;
    if (id == "Language")
    {
      auto available = value<C::Vector<std::string>>("Game", "available_locales");
      for (const std::string& a : available)
        possible_values.push_back (value<C::String>("Locale_" + a , "description"));
    }
    else if (id == "Fullscreen")
      possible_values = { "Yes", "No" };
    else if (id == "Interface_scale")
      possible_values = { "Tiny", "Small", "Large", "Huge" };
    else if (id == "Text_speed")
      possible_values = { "Slow", "Medium_speed", "Fast" };
    else if (id == "Music_volume" || id == "Sound_volume")
      possible_values = { "0", "10", "20", "30", "40",
                          "50", "60", "70", "80", "90",
                          "100" };

    auto pos = set<C::Relative_position>(id , "position", reference,
                                         Vector (Config::settings_menu_margin
                                                 + Config::settings_menu_in_margin,
                                                 y + Config::settings_menu_value_margin));
    for (std::size_t i = 0; i < possible_values.size(); ++ i)
    {
      std::string value_id = id + '_' + possible_values[i];
      std::string text;
      if (auto t = request<C::String>(possible_values[i] , "text"))
        text = locale(t->value());
      else if (is_int(possible_values[i]))
        text = possible_values[i] + " %";
      else
        text = possible_values[i];

      auto img = set<C::Image>(value_id , "image", light_font, "FFFFFF", text);
      img->z() = Config::menu_text_depth;
      img->on() = false;
      img->set_scale(0.45);
      img->set_relative_origin(0, 0.5);
      img->set_collision(UNCLICKABLE);
      set<C::Variable>(value_id , "position", pos);
      if (i == 0)
        node[1].init(img, pos);
      else
        node[1].add(img);
    }
  }

  // Arrows
  {
    auto left_arrow = set<C::Image>(id + "_left_arrow", "image",
                                    get<C::Image>("Menu_left_arrow", "image"));
    left_arrow->z() = Config::menu_text_depth;
    left_arrow->set_relative_origin(0.5, 0.5);
    left_arrow->set_collision(BOX);
    auto left_pos = set<C::Relative_position>(id + "_left_arrow", "position",
                                              reference,
                                              Vector(Config::settings_menu_larrow_x,
                                                     y + Config::settings_menu_value_margin
                                                     - Config::settings_menu_margin));
    node[2].init (left_arrow, left_pos);

    auto right_arrow = set<C::Image>(id + "_right_arrow", "image",
                                    get<C::Image>("Menu_right_arrow", "image"));
    right_arrow->z() = Config::menu_text_depth;
    right_arrow->set_relative_origin(0.5, 0.5);
    right_arrow->set_collision(BOX);
    auto right_pos = set<C::Relative_position>(id + "_right_arrow", "position",
                                               reference,
                                               Vector(Config::settings_menu_rarrow_x,
                                                      y + Config::settings_menu_value_margin
                                                      - Config::settings_menu_margin));
    node[3].init (right_arrow, right_pos);
  }
}

std::pair<C::Image_handle, C::Position_handle>
Menu::make_settings_button (const std::string& id, int y)
{
  auto button = request<C::Image>(id + "_button", "image");
  if (!button)
  {
    auto reference = get<C::Position>("Menu", "reference");
    auto button = set<C::Image>(id + "_button", "image",
                                get<C::Image>("Menu_settings_button", "image"));
    button->z() = Config::menu_button_depth;
    button->set_relative_origin(0.5, 0.5);
    button->on() = false;
    auto pos_button = set<C::Relative_position>(id + "_button", "position",
                                                reference,
                                                Vector (240, y + Config::settings_menu_height / 2));
    return std::make_pair (button, pos_button);
  }
  // else
  return std::make_pair (button, get<C::Position>(id + "_button", "position"));
}

std::pair<C::Image_handle, C::Position_handle>
Menu::make_settings_title (const std::string& id, const std::string& suffix, int y, const std::string& text)
{
  auto reference = get<C::Position>("Menu", "reference");
  auto font = get<C::Font>("Interface", "font");
  auto img = request<C::Image>(id , "image");
  C::Position_handle pos, pos_icon;
  if (!img)
  {
    img = set<C::Image>(id + "_" + suffix, "image", font, "FFFFFF", text);
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(0.5);
    if (img->width() * img->scale() > 415)
      img->set_scale(415 / double(img->width()));

    img->set_relative_origin(0, 0.5);
    img->set_collision(UNCLICKABLE);

    set<C::Relative_position>(id + "_" + suffix, "position", reference,
                              Vector (Config::settings_menu_margin
                                      + Config::settings_menu_in_margin,
                                      y + Config::settings_menu_in_margin
                                      + Config::settings_menu_margin));
  }
  else
    pos = get<C::Position>(id + "_" + suffix, "position");

  return std::make_pair (img, pos);
}

std::pair<C::Image_handle, C::Position_handle>
Menu::make_settings_subtitle (const std::string& id, const std::string& suffix,
                                   int y, const std::string& text)
{
  auto reference = get<C::Position>("Menu", "reference");
  auto light_font = get<C::Font>("Interface", "light_font");
  auto img = request<C::Image>(id + "_number", "image");
  C::Position_handle pos;
  if (!img)
  {
    img = set<C::Image>(id + "_" + suffix, "image", light_font, "FFFFFF", text);
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(0.45);
    if (img->width() * img->scale() > 415)
      img->set_scale(415 / double(img->width()));

    img->set_relative_origin(0, 0.5);
    img->set_collision(UNCLICKABLE);

    pos = set<C::Relative_position>(id + "_" + suffix, "position", reference,
                                    Vector (Config::settings_menu_margin
                                            + Config::settings_menu_in_margin,
                                            y + Config::settings_menu_value_margin));
  }
  else
    pos = get<C::Position>(id + "_" + suffix, "position");

  return std::make_pair (img, pos);
}



} // namespace Sosage::System
