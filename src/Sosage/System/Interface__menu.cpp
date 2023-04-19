/*
  [src/Sosage/System/Interface__menu.cpp]
  Handles menus.

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
#include <Sosage/System/Interface.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/datetime.h>

#include <queue>

namespace Sosage::Config
{
auto exit_menu_items
= {  "Phone", "Settings", "Credits", "New_game"
     #ifndef SOSAGE_EMSCRIPTEN
     , "Load", "Save", "Quit"
     #endif
  };
auto menus
= {  "Exit", "Phone", "Settings", "Credits"
     #ifndef SOSAGE_EMSCRIPTEN
     , "Load", "Save"
     #endif
  };

constexpr int menu_margin = 90;
constexpr int menu_small_margin = 35;
constexpr int menu_oknotok_y = 858;
constexpr int menu_ok_x = 120;
constexpr int menu_notok_x = 360;
constexpr int exit_menu_logo = 130;
constexpr int exit_menu_start = 220;
constexpr int exit_menu_text = menu_margin + 60;
constexpr int settings_menu_margin = 15;
constexpr int settings_menu_start = 135;
constexpr int settings_menu_height = 100;
constexpr int settings_menu_in_margin = 20;
constexpr int settings_menu_value_margin = 75;
constexpr int settings_menu_larrow_x = settings_menu_margin + 370;
constexpr int settings_menu_rarrow_x = settings_menu_margin + 420;
} // namespace Sosage::Config

namespace Sosage::System
{

namespace C = Component;

void Interface::init_menus()
{
  auto exit_menu = set<C::Menu>("Exit", "menu");

  exit_menu->split(VERTICALLY, Config::exit_menu_items.size() + 2);

  set<C::Relative_position>("Menu", "reference", get<C::Position>("Menu_background", "position"), Vector(-240, -420));
  make_exit_menu_item ((*exit_menu)[0], "Menu_logo", Config::exit_menu_logo);

  int y = Config::exit_menu_start;
  std::size_t idx = 1;
  for (const std::string& id : Config::exit_menu_items)
  {
    make_exit_menu_item ((*exit_menu)[idx], id, y);
    y += Config::menu_margin;
    idx ++;
  }

  make_oknotok_item ((*exit_menu)[idx], true);

  auto wanna_restart = set<C::Menu>("Wanna_restart", "menu");
  wanna_restart->split(VERTICALLY, 3);
  make_text_menu_title((*wanna_restart)[0], "New_game");
  make_text_menu_text((*wanna_restart)[1], "Wanna_restart");
  make_oknotok_item ((*wanna_restart)[2], false);

  auto wanna_save = set<C::Menu>("Wanna_save", "menu");
  wanna_save->split(VERTICALLY, 3);
  make_text_menu_title((*wanna_save)[0], "Save");
  make_text_menu_text((*wanna_save)[1], "Wanna_save");
  make_oknotok_item ((*wanna_save)[2], false);

  auto wanna_load = set<C::Menu>("Wanna_load", "menu");
  wanna_load->split(VERTICALLY, 3);
  make_text_menu_title((*wanna_load)[0], "Load");
  make_text_menu_text((*wanna_load)[1], "Wanna_load");
  make_oknotok_item ((*wanna_load)[2], false);

  auto saved = set<C::Menu>("Saved", "menu");
  saved->split(VERTICALLY, 3);
  make_text_menu_title((*saved)[0], "Save");
  make_text_menu_text((*saved)[1], "Saved");
  make_oknotok_item ((*saved)[2], true);

  auto credits_menu = set<C::Menu>("Credits", "menu");
  credits_menu->split(VERTICALLY, 3);
  make_text_menu_title((*credits_menu)[0], "Credits");
  make_text_menu_text((*credits_menu)[1], "Credits_text", true);
  make_oknotok_item ((*credits_menu)[2], true);

  auto phone_menu = set<C::Menu>("Phone", "menu");
  phone_menu->split(VERTICALLY, 3);
  make_text_menu_title((*phone_menu)[0], "Phone");
  make_text_menu_text((*phone_menu)[1], "No_number_text");
  make_oknotok_item ((*phone_menu)[2], true);

  init_loadsave_menus();
  set<C::Menu>("Message", "menu");

  auto settings_menu = set<C::Menu>("Settings", "menu");
  if constexpr (Config::emscripten || Config::android)
      settings_menu->split(VERTICALLY, 7);
  else
  settings_menu->split(VERTICALLY, 8);

  make_text_menu_title((*settings_menu)[0], "Settings");
  idx = 1;
  y = Config::settings_menu_start;
  for (const std::string& id : { "Language",
#if !defined (SOSAGE_ANDROID) && !defined(SOSAGE_EMSCRIPTEN)
        "Fullscreen",
#endif
       "Text_size", "Text_speed", "Music_volume", "Sound_volume" })
  {
    make_settings_item ((*settings_menu)[idx], id, y);
    y += Config::settings_menu_height + Config::settings_menu_margin;
    idx ++;
  }
  make_oknotok_item ((*settings_menu)[idx], true);

  auto menu_overlay = set<C::Image>("Menu_overlay", "image", Config::world_width, Config::world_height, 0, 0, 0, 64);
  menu_overlay->on() = false;
  menu_overlay->z() = Config::overlay_depth;
  menu_overlay->set_collision(UNCLICKABLE);
  set<C::Absolute_position>("Menu_overlay", "position", Point(0,0));
}

void Interface::init_loadsave_menus ()
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

void Interface::make_exit_menu_item (Component::Menu::Node node, const std::string& id, int y)
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

void Interface::make_oknotok_item (Component::Menu::Node node, bool only_ok)
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


void Interface::make_text_menu_title (Component::Menu::Node node, const std::string& id)
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

void Interface::make_text_menu_text (Component::Menu::Node node, const std::string& id, bool credits)
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

void Interface::make_settings_item (Component::Menu::Node node, const std::string& id, int y)
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
    else if (id == "Text_size")
      possible_values = { "Small", "Medium", "Large" };
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
Interface::make_settings_button (const std::string& id, int y)
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
Interface::make_settings_title (const std::string& id, const std::string& suffix, int y, const std::string& text)
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
Interface::make_settings_subtitle (const std::string& id, const std::string& suffix,
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

void Interface::update_menu()
{
  SOSAGE_UPDATE_DBG_LOCATION("Interface::update_menu()");

  if (auto menu = request<C::String>("Menu", "create"))
  {
    show_menu (menu->value());
    remove("Menu", "create");
  }

  if (auto menu = request<C::String>("Menu", "delete"))
  {
    hide_menu (menu->value());
    remove("Menu", "delete");
  }

  if (receive("Saves", "have_changed"))
  {
    delete_menu ("Load");
    delete_menu ("Save");
    init_loadsave_menus();
  }

  if (!status()->is (IN_MENU))
    return;

  if (receive ("Menu", "clicked"))
    menu_clicked();

  if (!status()->is (IN_MENU))
    return;

  bool gamepad = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD;
  bool touchscreen = value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == TOUCHSCREEN;

  const std::string& id = value<C::String>("Game", "current_menu");
  auto menu = get<C::Menu>(id , "menu");
  bool settings = (id == "Settings");

  std::string active_item = "";
  std::string setting_item = "";
  if (auto active = request<C::String>("Interface", "active_menu_item"))
  {
    active_item = active->value();
    setting_item = active_item;
    std::size_t pos = setting_item.find("_left_arrow");
    if (pos != std::string::npos)
      setting_item.resize(pos);
    else
    {
      pos = setting_item.find("_right_arrow");
      if (pos != std::string::npos)
        setting_item.resize(pos);
    }
  }

  std::queue<C::Menu::Node> todo;
  todo.push (menu->root());
  while (!todo.empty())
  {
    C::Menu::Node current = todo.front();
    todo.pop();

    if (current.has_image())
    {
      std::string entity = current.image()->entity();
      if (gamepad && active_item == "")
      {
        if (settings)
        {
          std::size_t pos = entity.find("_left_arrow");
          if (pos != std::string::npos)
            active_item = std::string(entity.begin(), entity.begin() + pos);
        }
        else if (contains (entity, "_button"))
          active_item = entity;

        set<C::String>("Interface", "active_menu_item", active_item);
        set<C::String>("Interface", "gamepad_active_menu_item", active_item);
      }

      bool active = (entity == active_item);
      if (settings && setting_item != "")
        active = active || contains(entity, setting_item);

      // Do not highlight items when using touchscreen
      if (touchscreen)
        active = false;

      current.image()->on() = true;
      if (contains(entity, "_button"))
        current.image()->set_alpha (active ? 255 : 0);
    }
    for (std::size_t i = 0; i < current.nb_children(); ++ i)
      todo.push (current[i]);
  }
}

void Interface::show_menu (const std::string& id)
{
  set<C::String>("Game", "current_menu", id);

  auto menu = get<C::Menu>(id , "menu");

  // Update settings menu with current settings
  if (id == "Settings")
  {
    menu->update_setting ("Language",
                          value<C::String>("Locale_" + value<C::String>(GAME__CURRENT_LOCAL) , "description"));

    menu->update_setting ("Fullscreen",
                          value<C::Boolean>("Window", "fullscreen") ? "Yes" : "No");

    int speed = value<C::Int>("Dialog", "speed");
    if (speed == Config::SLOW)
      menu->update_setting ("Text_speed", "Slow");
    else if (speed == Config::MEDIUM_SPEED)
      menu->update_setting ("Text_speed", "Medium_speed");
    else if (speed == Config::FAST)
      menu->update_setting ("Text_speed", "Fast");

    int size = value<C::Int>("Dialog", "size");
    if (size == Config::SMALL)
      menu->update_setting ("Text_size", "Small");
    else if (size == Config::MEDIUM)
      menu->update_setting ("Text_size", "Medium");
    else if (size == Config::LARGE)
      menu->update_setting ("Text_size", "Large");

    menu->update_setting ("Music_volume", std::to_string(10 * value<C::Int>("Music", "volume")));
    menu->update_setting ("Sound_volume", std::to_string(10 * value<C::Int>("Sounds", "volume")));
  }
  else if (id == "Message")
  {
    menu = set<C::Menu>("Message", "menu");
    menu->split(VERTICALLY, 2);
    make_text_menu_text((*menu)[0], "Message");
    make_oknotok_item ((*menu)[1], true);
  }
  else if (id == "Phone")
    update_phone_menu();

  get<C::Image>("Menu_background", "image")->on() = true;
  get<C::Image>("Menu_overlay", "image")->on() = true;
}

void Interface::hide_menu (const std::string& id)
{
  auto menu = get<C::Menu>(id , "menu");
  menu->hide();
  get<C::Image>("Menu_background", "image")->on() = false;
  get<C::Image>("Menu_overlay", "image")->on() = false;

  remove ("Interface", "active_menu_item", true);
  remove ("Interface", "gamepad_active_menu_item", true);
}

void Interface::delete_menu (const std::string& id)
{
  auto menu = get<C::Menu>(id , "menu");
  menu->apply([&](C::Image_handle img)
  {
    std::string id = img->entity();
    if (contains(id, "Exit_"))
      id = std::string (id.begin() + 5, id.end());
    if (request<C::String>(id , "text"))
      remove(img->entity(), img->component(), true);
  });
}

void Interface::menu_clicked ()
{
  std::string entity = value<C::String>("Interface", "active_menu_item");

  std::size_t pos = entity.find("_button");
  if (pos != std::string::npos)
    entity.resize(pos);

  const std::string& menu = value<C::String>("Game", "current_menu");

  auto effect = request<C::String>(entity , "effect");

  if (menu == "Settings")
  {
    if (!effect)
    {
      std::size_t pos = entity.find("_left_arrow");
      bool left_arrow = (pos != std::string::npos);
      if (!left_arrow)
      {
        pos = entity.find("_right_arrow");
        if (pos == std::string::npos)
          return;
      }

      std::string setting (entity.begin(), entity.begin() + pos);
      if (left_arrow)
        apply_setting (setting, get<C::Menu>("Settings", "menu")->decrement(setting));
      else
        apply_setting (setting, get<C::Menu>("Settings", "menu")->increment(setting));
      emit("Click", "play_sound");
    }
    else if (effect->value() != "Ok")
      return;
  }

  if (!effect)
    return;

  emit("Click", "play_sound");

  if (effect->value() == "Quit")
  {
    // Avoid exiting when testing input
    if (!signal ("Game", "prevent_exit"))
      emit ("Game", "exit");
    else
    {
      hide_menu(menu);
      status()->pop();
    }
    emit ("Game", "save");
    set<C::String>("Savegame", "id", "auto");
  }
  else if (effect->value() == "New_game")
  {
    hide_menu(menu);
    if (menu == "Exit")
      show_menu ("Wanna_restart");
    else
    {
      // Avoid restarting when testing input
      if (!signal ("Game", "prevent_restart"))
      {
        if (auto force = request<C::String>("Force_load", "room"))
        {
          set<C::Variable>("Game", "new_room", force);
          if (auto orig = request<C::String>("Force_load", "origin"))
            set<C::Variable>("Game", "new_room_origin", orig);
          else
            set<C::String>("Game", "new_room_origin", force->value() + "_test");
        }
        else
        {
          set<C::Variable>("Game", "new_room", get<C::String>("Game", "init_new_room"));
          set<C::Variable>("Game", "new_room_origin", get<C::String>("Game", "init_new_room_origin"));
        }
        emit ("Game", "reset");
        emit ("Music", "stop");
        status()->pop();
        status()->push(LOCKED);
      }
    }
  }
  else if (effect->value() == "Ok")
  {
    if (menu == "Wanna_restart")
    {
      // Avoid restarting when testing input
      if (!signal ("Game", "prevent_restart"))
      {
        if (auto force = request<C::String>("Force_load", "room"))
        {
          set<C::Variable>("Game", "new_room", force);
          if (auto orig = request<C::String>("Force_load", "origin"))
            set<C::Variable>("Game", "new_room_origin", orig);
          else
            set<C::String>("Game", "new_room_origin", force->value() + "_test");
        }
        else
        {
          set<C::Variable>("Game", "new_room", get<C::String>("Game", "init_new_room"));
          set<C::Variable>("Game", "new_room_origin", get<C::String>("Game", "init_new_room_origin"));
        }
        emit ("Game", "reset");
        emit ("Music", "stop");
        hide_menu("Wanna_restart");
        status()->pop();
        status()->push(LOCKED);
      }
    }
    else if (menu == "Wanna_save")
    {
      emit ("Game", "save");
      hide_menu(menu);
      show_menu("Saved");
    }
    else if (menu == "Wanna_load")
    {
      emit ("Game", "load");
      emit ("Game", "reset");
      hide_menu(menu);
      status()->pop();
    }
    else if (menu == "Credits" || menu == "Settings" || menu == "Phone" ||
             menu == "Save" || menu == "Load")
    {
      hide_menu (menu);
      show_menu ("Exit");
    }
    else if (menu == "Exit" || menu == "Message" || menu == "Saved")
    {
      hide_menu(menu);
      status()->pop();
    }
  }
  else if (effect->value() == "Credits" || effect->value() == "Settings"
           || effect->value() == "Phone" || effect->value() == "Save"
           || effect->value() == "Load")
  {
    hide_menu ("Exit");
    show_menu (effect->value());
  }
  else if (effect->value() == "Cancel")
  {
    hide_menu(menu);
    if (menu == "Exit")
      status()->pop();
    else if (menu == "Wanna_restart")
      show_menu("Exit");
    else if (menu == "Wanna_save")
      show_menu("Save");
    else if (menu == "Wanna_load")
      show_menu("Load");
  }
  else if (startswith(effect->value(), "Save_"))
  {
    std::string save_id (effect->value().begin() + effect->value().find('_') + 1, effect->value().end());
    set<C::String>("Savegame", "id", save_id);
    if (request<C::Tuple<std::string, double, int>>(effect->value(), "info"))
    {
      hide_menu(menu);
      show_menu("Wanna_save");
    }
    else
    {
      emit ("Game", "save");
      hide_menu(menu);
      show_menu("Saved");
    }
  }
  else if (startswith(effect->value(), "Load_"))
  {
    std::string save_id (effect->value().begin() + effect->value().find('_') + 1, effect->value().end());
    set<C::String>("Savegame", "id", save_id);
    hide_menu(menu);
    show_menu("Wanna_load");
  }
  else if (auto action = request<C::Action>(effect->value() , "action"))
  {
    hide_menu(menu);
    status()->pop();
    set<C::Variable>("Character", "triggered_action", action);
  }
  else
  {
    check(false, "Unknown menu effect: " + effect->value());
  }
}

void Interface::apply_setting (const std::string& setting, const std::string& v)
{
  debug << "Apply setting " << v << " to " << setting << std::endl;
  if (setting == "Language")
  {
    auto available = value<C::Vector<std::string>>("Game", "available_locales");
    for (const std::string& a : available)
      if (value<C::String>("Locale_" + a , "description") == v)
      {
        get<C::String>(GAME__CURRENT_LOCAL)->set(a);
        break;
      }

    // Delete all menus
    for (const std::string& id : Config::menus)
      if (id != "New_game")
        delete_menu(id);

    // Reinit interface
    init();
    show_menu ("Settings");

    // Update Game name
    emit("Game", "name_changed");
  }
  else if (setting == "Fullscreen")
  {
    // Avoid switching fullscreen when testing input
    if (!signal ("Game", "prevent_restart"))
    {
      get<C::Boolean>("Window", "fullscreen")->set(v == "Yes");
      emit ("Window", "toggle_fullscreen");
    }
  }
  else if (setting == "Text_size")
  {
    if (v == "Small")
      set<C::Int>("Dialog", "size")->set(Config::SMALL);
    else if (v == "Medium")
      set<C::Int>("Dialog", "size")->set(Config::MEDIUM);
    else
      set<C::Int>("Dialog", "size")->set(Config::LARGE);
  }
  else if (setting == "Text_speed")
  {
    if (v == "Slow")
      set<C::Int>("Dialog", "speed")->set(Config::SLOW);
    else if (v == "Medium_speed")
      set<C::Int>("Dialog", "speed")->set(Config::MEDIUM_SPEED);
    else
      set<C::Int>("Dialog", "speed")->set(Config::FAST);
  }
  else if (setting == "Music_volume")
  {
    if (!signal ("Game", "prevent_restart"))
    {
      set<C::Int>("Music", "volume")->set(to_int(v) / 10);
      emit("Music", "volume_changed");
    }
  }
  else if (setting == "Sound_volume")
  {
    if (!signal ("Game", "prevent_restart"))
      set<C::Int>("Sounds", "volume")->set(to_int(v) / 10);
  }
}

void Interface::update_phone_menu()
{
  // If not IDLE, can't use phone right now
  if (!status()->was(IDLE))
  {
    auto phone_menu = set<C::Menu>("Phone", "menu");
    phone_menu->split(VERTICALLY, 3);
    make_text_menu_title((*phone_menu)[0], "Phone");
    make_text_menu_text((*phone_menu)[1], "No_phone_allowed");
    make_oknotok_item ((*phone_menu)[2], true);
    return;
  }

  auto numbers = request<C::Vector<std::string>>("phone_numbers", "list");
  if (!numbers)
    return;

  auto phone_menu = get<C::Menu>("Phone", "menu");
  bool uptodate = true;

  if (phone_menu->nb_children() != numbers->value().size() + 2)
    uptodate = false;
  else
  {
    std::size_t idx = 1;
    for (const std::string& id : numbers->value())
    {
      if ((*phone_menu)[idx].nb_children() != 2)
      {
        uptodate = false;
        break;
      }
      else if (!(*phone_menu)[idx].has_image())
      {
        uptodate = false;
        break;
      }
      else if ((*phone_menu)[idx].image()->entity() != id + "_button")
      {
        uptodate = false;
        break;
      }
      else if (!request<C::Image>((*phone_menu)[idx].image()->entity(), (*phone_menu)[idx].image()->component()))
      {
        uptodate = false;
        break;
      }
      ++ idx;
    }
  }

  if (uptodate)
  {
    debug << "Phone book is up to date" << std::endl;
    return;
  }
  else
  {
    debug << "Phone book is NOT up to date, updating..." << std::endl;
  }

  phone_menu = set<C::Menu>("Phone", "menu");
  phone_menu->split(VERTICALLY, numbers->value().size() + 2);
  make_text_menu_title((*phone_menu)[0], "Phone");

  // Make action item

  std::size_t idx = 1;
  int y = Config::settings_menu_start;
  for (const std::string& id : numbers->value())
  {
    std::string label = locale_get(id , "label");
    std::size_t pos = label.find_last_of(' ');
    check(pos != std::string::npos, "Warning: ill-formed number " + label);

    std::string name (label.begin(), label.begin() + pos);
    std::string number (label.begin() + pos + 1, label.end());

    // Create button
    C::Menu::Node node = (*phone_menu)[idx];
    C::Image_handle button;
    C::Position_handle pos_button;
    std::tie (button, pos_button) = make_settings_button (id, y);
    node.init(button, pos_button);

    set<C::String>(id , "effect", id);

    node.split(VERTICALLY, 2);

    // Name
    {
      C::Image_handle img;
      C::Position_handle pos;
      std::tie (img, pos) = make_settings_title (id, "name", y, name);
      node[0].init(img, pos);
    }

    // Number value
    {
      C::Image_handle img;
      C::Position_handle pos;
      std::tie (img, pos) = make_settings_subtitle (id, "number", y, number);
      node[1].init(img, pos);
    }
    y += Config::settings_menu_height;
    ++ idx;
  }

  make_oknotok_item ((*phone_menu)[idx], true);
}

} // namespace Sosage::Config
