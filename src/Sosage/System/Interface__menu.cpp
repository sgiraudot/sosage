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

#include <Sosage/Component/Hints.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Interface.h>
#include <Sosage/Utils/color.h>

#include <queue>

namespace Sosage::Config
{
constexpr int menu_margin = 85;
constexpr int menu_small_margin = 35;
constexpr int menu_oknotok_y = 858;
constexpr int menu_ok_x = 120;
constexpr int menu_notok_x = 360;
constexpr int exit_menu_logo = 130;
constexpr int exit_menu_start = 300;
constexpr int exit_menu_text = menu_margin + 60;
constexpr int settings_menu_margin = 15;
constexpr int settings_menu_start = 120;
constexpr int settings_menu_in_margin = 20;
constexpr int settings_menu_value_margin = 65;
constexpr int settings_menu_larrow_x = settings_menu_margin + 370;
constexpr int settings_menu_rarrow_x = settings_menu_margin + 420;
}

namespace Sosage::System
{

namespace C = Component;

void Interface::init_menus()
{
  auto exit_menu = set<C::Menu>("Exit:menu");

  if constexpr (Config::emscripten)
    exit_menu->split(VERTICALLY, 7);
  else
    exit_menu->split(VERTICALLY, 8);

  set<C::Relative_position>("Menu:reference", get<C::Position>("Menu_background:position"), Vector(-240, -420));
  make_exit_menu_item ((*exit_menu)[0], "Menu_logo", Config::exit_menu_logo);

  int y = Config::exit_menu_start;
  std::size_t idx = 1;
  for (const std::string& id : { "New_game", "Settings", "Phone", "Gps", "Credits"
#ifndef SOSAGE_EMSCRIPTEN
       , "Save_and_quit"
#endif
})
  {
    make_exit_menu_item ((*exit_menu)[idx], id, y);
    y += Config::menu_margin;
    idx ++;
  }

  make_oknotok_item ((*exit_menu)[idx], true);

  auto wanna_restart = set<C::Menu>("Wanna_restart:menu");
  wanna_restart->split(VERTICALLY, 3);
  make_text_menu_title((*wanna_restart)[0], "New_game");
  make_text_menu_text((*wanna_restart)[1], "Wanna_restart");
  make_oknotok_item ((*wanna_restart)[2], false);

  auto credits_menu = set<C::Menu>("Credits:menu");
  credits_menu->split(VERTICALLY, 3);
  make_text_menu_title((*credits_menu)[0], "Credits");
  make_text_menu_text((*credits_menu)[1], "Credits_text");
  make_oknotok_item ((*credits_menu)[2], true);

  auto end_menu = set<C::Menu>("End:menu");
  end_menu->split(VERTICALLY, 2);
  make_text_menu_text((*end_menu)[0], "End_text");
  make_oknotok_item ((*end_menu)[1], true);

  auto settings_menu = set<C::Menu>("Settings:menu");
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
    y += Config::settings_menu_start;
    idx ++;
  }
  make_oknotok_item ((*settings_menu)[idx], true);

#if 0

  auto end_menu = set<C::Menu>("End:menu");
  end_menu->split(VERTICALLY, 2);
  (*end_menu)[1].split(HORIZONTALLY, 2);
  init_menu_item ((*end_menu)[0], "End_text", "");
  init_menu_item ((*end_menu)[1][0], "Restart", "new_game");
  init_menu_item ((*end_menu)[1][1], "Quit", "quit");
  init_menu_buttons ((*end_menu)[1]);
#endif
}

void Interface::make_exit_menu_item (Component::Menu::Node node, const std::string& id, int y)
{
  auto reference = get<C::Position>("Menu:reference");

  if (id == "Menu_logo")
  {
    auto img = get<C::Image>(id + ":image");
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_relative_origin(0.5, 0.5);
    auto pos = set<C::Relative_position>(id + ":position", reference, Vector(240, y));
    node.init(img, pos);
    return;
  }

  auto font = get<C::Font>("Interface:font");
  auto text = request<C::String>(id + ":text");

  node.split(BUTTON, 1);
  node[0].split(HORIZONTALLY, 2);

  // might be reused
  auto icon = get<C::Image>(id + "_icon:image");
  auto img = request<C::Image>("Exit_" + id + ":image");
  C::Position_handle pos, pos_icon;
  if (!img)
  {
    img = set<C::Image>("Exit_" + id + ":image", font, "FFFFFF", locale(text->value()));
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(0.5);
    img->set_relative_origin(0, 0.5);
    img->set_collision(UNCLICKABLE);

    pos = set<C::Relative_position>("Exit_" + id + ":position", reference, Vector (Config::exit_menu_text, y));
    pos_icon = set<C::Relative_position>(id + "_icon:position", reference, Vector (Config::menu_margin, y));
  }
  else
  {
    pos = get<C::Position>("Exit_" + id + ":position");
    icon = get<C::Image>(id + "_icon:image");
    pos_icon = get<C::Position>(id + "_icon:position");
  }
  node[0][0].init(icon, pos_icon);
  node[0][1].init(img, pos);

  // Create button
  auto button = request<C::Image>(id + "_button:image");
  C::Position_handle pos_button;
  if (!button)
  {
    button = set<C::Image>(id + "_button:image", get<C::Image>("Menu_main_button:image"));
    //button = set<C::Image>(id + "_button:image", Config::exit_menu_button_width, Config::exit_menu_button_height, 0, 0, 0, 64);
    button->z() = Config::menu_button_depth;
    button->set_relative_origin(0.5, 0.5);
    button->on() = false;
    pos_button = set<C::Relative_position>(id + "_button:position", reference, Vector (240, y));
  }
  else
    pos_button = get<C::Position>(id + "_button:position");
  node.init(button, pos_button);

  set<C::String>(id + ":effect", id);
}

void Interface::make_oknotok_item (Component::Menu::Node node, bool only_ok)
{
  C::Image_handle ok, cancel, ok_alone, ok_button, cancel_button, ok_alone_button;
  C::Position_handle ok_pos, cancel_pos, ok_alone_pos,
      ok_button_pos, cancel_button_pos, ok_alone_button_pos;

  ok_alone = request<C::Image>("Ok_alone_icon:image");

  // Only create once
  if (!ok)
  {
    auto reference = get<C::Position>("Menu:reference");

    ok = get<C::Image>("Ok_icon:image");
    ok->z() = Config::menu_text_depth;
    ok->on() = false;
    ok->set_relative_origin(0.5, 0.5);
    ok->set_collision(UNCLICKABLE);
    ok_alone = set<C::Image>("Ok_alone_icon:image", ok);

    ok_pos = set<C::Relative_position>("Ok_icon:position", reference,
                                       Vector (Config::menu_ok_x, Config::menu_oknotok_y));
    ok_alone_pos = set<C::Relative_position>("Ok_alone_icon:position", reference,
                                             Vector (240, Config::menu_oknotok_y));

    ok_button = set<C::Image>("Ok_button:image", 230, 50, 0, 0, 0, 64);
    ok_button->z() = Config::menu_button_depth;
    ok_button->set_relative_origin(0.5, 0.5);
    ok_button->on() = false;
    ok_alone_button = set<C::Image>("Ok_alone_button:image", 460, 50, 0, 0, 0, 64);
    ok_alone_button->z() = Config::menu_button_depth;
    ok_alone_button->set_relative_origin(0.5, 0.5);
    ok_alone_button->on() = false;
    ok_button_pos = set<C::Relative_position>("Ok_button:position", ok_pos, Vector(0,0));
    ok_alone_button_pos = set<C::Relative_position>("Ok_alone_button:position", ok_alone_pos, Vector(0,0));

    set<C::String>("Ok:effect", "Ok");
    set<C::String>("Ok_alone:effect", "Ok");

    cancel = get<C::Image>("Cancel_icon:image");
    cancel->z() = Config::menu_text_depth;
    cancel->on() = false;
    cancel->set_relative_origin(0.5, 0.5);
    cancel->set_collision(UNCLICKABLE);
    cancel_pos = set<C::Relative_position>("Cancel_icon:position", reference,
                                           Vector (Config::menu_notok_x, Config::menu_oknotok_y));

    cancel_button = set<C::Image>("Cancel_button:image", 230, 50, 0, 0, 0, 64);
    cancel_button->set_relative_origin(0.5, 0.5);
    cancel_button->on() = false;
    cancel_button_pos = set<C::Relative_position>("Cancel_button:position", cancel_pos, Vector(0,0));
    cancel_button->z() = Config::menu_button_depth;

    set<C::String>("Cancel:effect", "Cancel");
  }
  else
  {
    ok = get<C::Image>("Ok_icon:image");
    cancel = get<C::Image>("Cancel_icon:image");
    ok_button = get<C::Image>("Ok_button:image");
    cancel_button = get<C::Image>("Cancel_button:image");
    ok_alone_button = get<C::Image>("Ok_alone_button:image");
    ok_pos = get<C::Position>("Ok_icon:position");
    cancel_pos = get<C::Position>("Cancel_icon:position");
    ok_alone_pos = get<C::Position>("Ok_alone_icon:position");
    ok_button_pos = get<C::Position>("Ok_button:position");
    cancel_button_pos = get<C::Position>("Cancel_button:position");
    ok_alone_button_pos = get<C::Position>("Ok_alone_button:position");
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
  auto reference = get<C::Position>("Menu:reference");
  auto font = get<C::Font>("Interface:font");
  auto text = get<C::String>(id + ":text");
  auto img = set<C::Image>("Title_" + id + ":image", font, "FFFFFF", locale(text->value()));
  img->z() = Config::menu_text_depth;
  img->on() = false;
  img->set_scale(0.75);
  img->set_relative_origin(0.5, 0.5);
  img->set_collision(UNCLICKABLE);
  auto pos = set<C::Relative_position>("Title_" + id + ":position", reference,
                                       Point(240, Config::menu_margin));
  node.init(img, pos);
}

void Interface::make_text_menu_text (Component::Menu::Node node, const std::string& id)
{
  auto reference = get<C::Position>("Menu:reference");
  auto font = get<C::Font>("Interface:light_font");
  auto text = get<C::String>(id + ":text");

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

  double scale = 0.5;
  node.split(VERTICALLY, lines.size());
  int y = Config::exit_menu_start;

  for (std::size_t i = 0; i < lines.size(); ++ i)
  {
    auto img = set<C::Image>(text->entity() + "_" + std::to_string(i)
                             + ":image", font, "FFFFFF", lines[i]);
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(scale);
    img->set_relative_origin(0, 0);
    img->set_collision(UNCLICKABLE);
    auto pos = set<C::Relative_position>(text->entity() + "_" + std::to_string(i)
                                         + ":position", reference,
                                         Point(Config::menu_small_margin, y));
    node[i].init(img, pos);
    y += Config::menu_small_margin;
  }
}

void Interface::make_settings_item (Component::Menu::Node node, const std::string& id, int y)
{
  auto reference = get<C::Position>("Menu:reference");
  auto font = get<C::Font>("Interface:font");
  auto light_font = get<C::Font>("Interface:light_font");

  // Create button
  auto button = request<C::Image>(id + "_button:image");
  C::Position_handle pos_button;
  if (!button)
  {
    button = set<C::Image>(id + "_button:image", get<C::Image>("Menu_settings_button:image"));
    button->z() = Config::menu_button_depth;
    button->set_relative_origin(0.5, 0.5);
    button->on() = false;
    pos_button = set<C::Relative_position>(id + "_button:position", reference,
                                           Vector (240, y + Config::settings_menu_start / 2 - Config::settings_menu_margin));
  }
  else
    pos_button = get<C::Position>(id + "_button:position");
  node.init(button, pos_button);

  set<C::String>(id + ":effect", id);

  node.split(HORIZONTALLY, 4);

  // Setting title
  {
    auto text = request<C::String>(id + ":text");
    auto img = request<C::Image>(id + ":image");
    C::Position_handle pos, pos_icon;
    if (!img)
    {
      img = set<C::Image>(id + "_setting:image", font, "FFFFFF", locale(text->value()));
      img->z() = Config::menu_text_depth;
      img->on() = false;
      img->set_scale(0.5);
      img->set_relative_origin(0, 0.5);
      img->set_collision(UNCLICKABLE);

      pos = set<C::Relative_position>(id + "_setting:position", reference,
                                      Vector (Config::settings_menu_margin + Config::settings_menu_in_margin,
                                              y + Config::settings_menu_in_margin));
    }
    else
    {
      pos = get<C::Position>(id + "_setting:position");
    }
    node[0].init(img, pos);
  }

  // Setting value
  {
    std::vector<std::string> possible_values;
    if (id == "Language")
    {
      auto available = get<C::Vector<std::string>>("Game:available_locales")->value();
      for (const std::string& a : available)
        possible_values.push_back (get<C::String>(a + ":description")->value());
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

    auto pos = set<C::Relative_position>(id + ":position", reference,
                                         Vector (Config::settings_menu_margin + Config::settings_menu_in_margin,
                                                 y + Config::settings_menu_value_margin));
    for (std::size_t i = 0; i < possible_values.size(); ++ i)
    {
      std::string value_id = id + '_' + possible_values[i];
      std::string text;
      if (auto t = request<C::String>(possible_values[i] + ":text"))
        text = locale(t->value());
      else if (is_int(possible_values[i]))
        text = possible_values[i] + " %";
      else
        text = possible_values[i];

      auto img = set<C::Image>(value_id + ":image", light_font, "FFFFFF", text);
      img->z() = Config::menu_text_depth;
      img->on() = false;
      img->set_scale(0.5);
      img->set_relative_origin(0, 0.5);
      img->set_collision(UNCLICKABLE);
      set<C::Variable>(value_id + ":position", pos);
      if (i == 0)
        node[1].init(img, pos);
      else
        node[1].add(img);
    }
  }

  // Arrows
  {
    auto left_arrow = set<C::Image>(id + "_left_arrow:image",
                                    get<C::Image>("Menu_left_arrow:image"));
    left_arrow->z() = Config::menu_text_depth;
    left_arrow->set_relative_origin(0.5, 0.5);
    left_arrow->set_collision(BOX);
    auto left_pos = set<C::Relative_position>(id + "_left_arrow:position",
                                              reference,
                                              Vector(Config::settings_menu_larrow_x,
                                                     y + Config::settings_menu_start / 2));
    node[2].init (left_arrow, left_pos);

    auto right_arrow = set<C::Image>(id + "_right_arrow:image",
                                    get<C::Image>("Menu_right_arrow:image"));
    right_arrow->z() = Config::menu_text_depth;
    right_arrow->set_relative_origin(0.5, 0.5);
    right_arrow->set_collision(BOX);
    auto right_pos = set<C::Relative_position>(id + "_right_arrow:position",
                                               reference,
                                               Vector(Config::settings_menu_rarrow_x,
                                                      y + Config::settings_menu_start / 2));
    node[3].init (right_arrow, right_pos);
  }
}

void Interface::init_menu_item (Component::Menu::Node node, const std::string& id,
                               const std::string& effect)
{
#if 0
  auto font = get<C::Font>("Interface:font");
  auto light_font = get<C::Font>("Interface:font");

  auto text = request<C::String>(id + ":text");

  if (effect == "")
  {
    if (text)
    {
      const std::string& str = locale(text->value());
      std::size_t pos = str.find('\n');
      if (pos == std::string::npos)
      {
        auto img = set<C::Image>(text->entity() + ":image",
                                 font, "FFFFFF", locale(text->value()));
        img->z() = Config::menu_text_depth;
        img->on() = false;
        img->set_scale(0.75);
        img->set_relative_origin(0.5, 0.5);
        img->set_collision(UNCLICKABLE);
        auto pos = set<C::Absolute_position>(text->entity() + ":position", Point(0,0));
        node.init(img, pos);
      }
      else
      {
        std::vector<std::string> lines;
        std::size_t begin = 0;
        do
        {
          lines.emplace_back (str.begin() + begin, str.begin() + pos);
          begin = pos + 1;
          pos = str.find('\n', pos+2);
        }
        while (pos != std::string::npos);

        double scale = 0.75;
        if (lines.size() > 2) // hack for cursor selection menu
          scale = 0.6;

        node.split(VERTICALLY, lines.size());
        for (std::size_t i = 0; i < lines.size(); ++ i)
        {
          auto img = set<C::Image>(text->entity() + "_" + std::to_string(i)
                                   + ":image",
                                   interface_font, "000000", lines[i]);
          img->z() = Config::menu_text_depth;
          img->on() = false;
          img->set_scale(scale);
          img->set_relative_origin(0.5, 0.5);
          img->set_collision(UNCLICKABLE);
          auto pos = set<C::Absolute_position>(text->entity() + "_" + std::to_string(i)
                                      + ":position", Point(0, 0));
          node[i].init(img, pos);
        }
      }
    }
    else // image
    {
      auto img = get<C::Image>(id + ":image");
      img->z() = Config::menu_text_depth;
      img->on() = false;
      img->set_relative_origin(0.5, 0.5);
      auto pos = set<C::Absolute_position>(id + ":position", Point(0,0));
      node.init(img, pos);
    }
  }
  else // button
  {
    node.split(BUTTON, 1);
    // might be reused
    auto img = request<C::Image>(text->entity() + ":image");
    C::Position_handle pos;
    if (!img)
    {
      img = set<C::Image>(text->entity() + ":image",
                          dialog_font, menu_color, locale(text->value()));
      img->z() = Config::menu_text_depth;
      img->on() = false;
      img->set_scale(0.75);
      img->set_relative_origin(0.5, 0.5);
      img->set_collision(UNCLICKABLE);

      pos = set<C::Absolute_position>(text->entity() + ":position", Point(0,0));
    }
    else
      pos = get<C::Position>(text->entity() + ":position");
    node[0].init(img, pos);

    set<C::String>(text->entity() + ":effect", effect);
  }
#endif
}

void Interface::init_setting_item (Component::Menu::Node node_left,
                                   Component::Menu::Node node,
                                   Component::Menu::Node node_right,
                                   const std::string& effect)
{
  auto left_arrow = set<C::Image>("Menu_" + effect + "_left_arrow:image",
                                  get<C::Image>("Menu_left_arrow:image"));
  left_arrow->z() = Config::menu_text_depth;
  left_arrow->set_relative_origin(0.5, 0.5);
  left_arrow->set_collision(BOX);
  auto left_pos = set<C::Absolute_position>("Menu_" + effect + "_left_arrow:position", Point(0, 0));
  node_left.init (left_arrow, left_pos);
  auto right_arrow = set<C::Image>("Menu_" + effect + "_right_arrow:image",
                                  get<C::Image>("Menu_right_arrow:image"));
  right_arrow->z() = Config::menu_text_depth;
  right_arrow->set_relative_origin(0.5, 0.5);
  right_arrow->set_collision(BOX);
  auto right_pos = set<C::Absolute_position>("Menu_" + effect + "_right_arrow:position", Point(0, 0));
  node_right.init (right_arrow, right_pos);

  std::vector<std::string> possible_values;
  if (effect == "language")
  {
    auto available = get<C::Vector<std::string>>("Game:available_locales")->value();
    for (const std::string& a : available)
      possible_values.push_back (get<C::String>(a + ":description")->value());
  }
  else if (effect == "fullscreen")
    possible_values = { "Yes", "No" };
  else if (effect == "text_size")
    possible_values = { "Small", "Medium", "Large" };
  else if (effect == "text_speed")
    possible_values = { "Slow", "Medium_speed", "Fast" };
  else if (effect == "music_volume" || effect == "sound_volume")
    possible_values = { "0", "10", "20", "30", "40",
                        "50", "60", "70", "80", "90",
                        "100" };

  auto menu_font = get<C::Font>("Interface:font");

  auto pos = set<C::Absolute_position>(effect + ":position", Point(0,0));
  for (std::size_t i = 0; i < possible_values.size(); ++ i)
  {
    std::string id = effect + '_' + possible_values[i];
    std::string text;
    if (auto t = request<C::String>(possible_values[i] + ":text"))
      text = locale(t->value());
    else if (is_int(possible_values[i]))
      text = possible_values[i] + " %";
    else
      text = possible_values[i];

    auto img = set<C::Image>(id + ":image",
                             menu_font, "000000", text);
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(0.75);
    img->set_relative_origin(0.5, 0.5);
    img->set_collision(UNCLICKABLE);
    set<C::Variable>(id + ":position", pos);
    if (i == 0)
      node.init(img, pos);
    else
      node.add(img);
  }

}

void Interface::init_menu_buttons (Component::Menu::Node node)
{
  if (node.direction() == VERTICALLY)
  {
    double width = 0;
    for (std::size_t i = 0; i < node.nb_children(); ++ i)
      if (node[i].direction() == BUTTON)
        width = std::max (width, node[i][0].size().x());

    for (std::size_t i = 0; i < node.nb_children(); ++ i)
      if (node[i].direction() == BUTTON)
      {
        auto img = request<C::Image>(node[i][0].image()->entity() + "_button:image");
        C::Position_handle pos;
        if (!img)
        {
          img = set<C::Image>(node[i][0].image()->entity() + "_button:image",
              width, node[i][0].size().y());
          img->z() = Config::menu_button_depth;
          img->set_relative_origin(0.5, 0.5);
          img->on() = false;
          pos = set<C::Absolute_position>(img->entity() + ":position", Point(0,0));
        }
        else
          pos = get<C::Position>(img->entity() + ":position");
        node[i].init(img, pos);
      }
  }
  else
  {
    check (node.direction() == HORIZONTALLY, "Menu node should be vertical or horizontal");
    double height = 0;
    for (std::size_t i = 0; i < node.nb_children(); ++ i)
      if (node[i].direction() == BUTTON)
        height = std::max (height, node[i][0].size().y());
    for (std::size_t i = 0; i < node.nb_children(); ++ i)
      if (node[i].direction() == BUTTON)
      {
        auto img = request<C::Image>(node[i][0].image()->entity() + "_button:image");
        C::Position_handle pos;
        if (!img)
        {
          img = set<C::Image>(node[i][0].image()->entity() + "_button:image",
              node[i][0].size().x(), height);
          img->z() = Config::menu_button_depth;
          img->set_relative_origin(0.5, 0.5);
          img->on() = false;
          pos = set<C::Absolute_position>(img->entity() + ":position", Point(0,0));
        }
        else
          pos = get<C::Position>(img->entity() + ":position");
        node[i].init(img, pos);
      }
  }
}

void Interface::update_menu()
{
  if (auto menu = request<C::String>("Menu:create"))
  {
    create_menu (menu->value());
    remove("Menu:create");
  }

  if (auto menu = request<C::String>("Menu:delete"))
  {
    delete_menu (menu->value());
    remove("Menu:delete");
  }

  if (!status()->is (IN_MENU))
    return;

  if (receive ("Menu:clicked"))
    menu_clicked();

  if (!status()->is (IN_MENU))
    return;

  bool gamepad = get<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE)->value() == GAMEPAD;

  const std::string& id = get<C::String>("Game:current_menu")->value();
  auto menu = get<C::Menu>(id + ":menu");
  bool settings = (id == "Settings");

  std::string active_item = "";
  std::string setting_item = "";
  if (auto active = request<C::String>("Interface:active_menu_item"))
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

        set<C::String>("Interface:active_menu_item", active_item);
        set<C::String>("Interface:gamepad_active_menu_item", active_item);
      }

      bool active = (entity == active_item);
      if (settings && setting_item != "")
        active = active || contains(entity, setting_item);

      current.image()->on() = true;
      if (contains(entity, "_button"))
        current.image()->set_alpha (active ? 255 : 0);
    }
    for (std::size_t i = 0; i < current.nb_children(); ++ i)
      todo.push (current[i]);
  }
}

void Interface::create_menu (const std::string& id)
{
  set<C::String>("Game:current_menu", id);

  // Hint menu is created on the fly
  if (id == "Hint")
  {
    std::string hint = get<C::Hints>("Game:hints")->next();
    if (hint == "")
      hint = get<C::String>("No_hint:text")->value();
    set<C::String>("Hint_text:text", hint);
    auto hint_menu = set<C::Menu>("Hint:menu");
    hint_menu->split(VERTICALLY, 2);
    init_menu_item ((*hint_menu)[0], "Hint_text", "");
    init_menu_item ((*hint_menu)[1], "Ok", "ok");
    init_menu_buttons (hint_menu->root());
  }

  auto menu = get<C::Menu>(id + ":menu");

  // Update settings menu with current settings
  if (id == "Settings")
  {
    menu->update_setting ("Language",
                          get<C::String>(get<C::String>(GAME__CURRENT_LOCAL)->value() + ":description")->value());

    menu->update_setting ("Fullscreen",
                          get<C::Boolean>("Window:fullscreen")->value() ? "Yes" : "No");

    int speed = get<C::Int>("Dialog:speed")->value();
    if (speed == Config::SLOW)
      menu->update_setting ("Text_speed", "Slow");
    else if (speed == Config::MEDIUM_SPEED)
      menu->update_setting ("Text_speed", "Medium_speed");
    else if (speed == Config::FAST)
      menu->update_setting ("Text_speed", "Fast");

    int size = get<C::Int>("Dialog:size")->value();
    if (size == Config::SMALL)
      menu->update_setting ("Text_size", "Small");
    else if (size == Config::MEDIUM)
      menu->update_setting ("Text_size", "Medium");
    else if (size == Config::LARGE)
      menu->update_setting ("Text_size", "Large");

    menu->update_setting ("Music_volume", std::to_string(10 * get<C::Int>("Music:volume")->value()));
    menu->update_setting ("Sound_volume", std::to_string(10 * get<C::Int>("Sounds:volume")->value()));
  }

  get<C::Image>("Menu_background:image")->on() = true;
}

void Interface::delete_menu (const std::string& id)
{
  auto menu = get<C::Menu>(id + ":menu");
  menu->hide();
  get<C::Image>("Menu_background:image")->on() = false;

  remove ("Interface:active_menu_item", true);
  remove ("Interface:gamepad_active_menu_item", true);
}

void Interface::menu_clicked ()
{
  std::string entity = get<C::String>("Interface:active_menu_item")->value();

  std::size_t pos = entity.find("_button");
  if (pos != std::string::npos)
    entity.resize(pos);

  const std::string& menu = get<C::String>("Game:current_menu")->value();

  auto effect = request<C::String>(entity + ":effect");

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
        apply_setting (setting, get<C::Menu>("Settings:menu")->decrement(setting));
      else
        apply_setting (setting, get<C::Menu>("Settings:menu")->increment(setting));
      emit("Click:play_sound");
    }
    else if (effect->value() != "Ok")
      return;
  }

  if (!effect)
    return;

  emit("Click:play_sound");

  if (effect->value() == "Quit")
  {
    if (menu != "End")
      emit ("Game:save");
    emit ("Game:exit");
  }
  else if (effect->value() == "New_game")
  {
    delete_menu(menu);
    if (menu == "Exit")
      create_menu ("Wanna_restart");
    else
    {
      set<C::Variable>("Game:new_room", get<C::String>("Game:init_new_room"));
      if (auto orig = request<C::String>("Game:init_new_room_origin"))
        set<C::Variable>("Game:new_room_origin", orig);
      emit ("Game:reset");
      remove("Game:music");
      emit ("Music:stop");
      status()->pop();
    }
  }
  else if (effect->value() == "Ok")
  {
    if (menu == "Wanna_restart")
    {
      set<C::Variable>("Game:new_room", get<C::String>("Game:init_new_room"));
      if (auto orig = request<C::String>("Game:init_new_room_origin"))
        set<C::Variable>("Game:new_room_origin", orig);
      emit ("Game:reset");
      remove("Game:music");
      emit ("Music:stop");
      delete_menu("Wanna_restart");
      status()->pop();
    }
    else if (menu == "Credits" || menu == "Settings")
    {
      delete_menu (menu);
      create_menu ("Exit");
    }
    else if (menu == "Hint")
    {
      delete_menu("Hint");
      status()->pop();
    }
    else if (menu == "Exit")
    {
      delete_menu(menu);
      status()->pop();
    }
  }
  else if (effect->value() == "Settings")
  {
    delete_menu ("Exit");
    create_menu ("Settings");
  }
  else if (effect->value() == "Credits")
  {
    delete_menu ("Exit");
    create_menu ("Credits");
  }
  else if (effect->value() == "Hint")
  {
    delete_menu ("Exit");
    create_menu ("Hint");
  }
  else if (effect->value() == "Cancel")
  {
    delete_menu(menu);
    if (menu == "Exit")
      status()->pop();
    else if (menu == "Wanna_restart")
      create_menu("Exit");
  }
}

void Interface::apply_setting (const std::string& setting, const std::string& value)
{
  std::cerr << "Apply setting " << value << " to " << setting << std::endl;
  if (setting == "Language")
  {
    auto available = get<C::Vector<std::string>>("Game:available_locales")->value();
    for (const std::string& a : available)
      if (get<C::String>(a + ":description")->value() == value)
      {
        get<C::String>(GAME__CURRENT_LOCAL)->set(a);
        break;
      }

    // Delete all menus
    for (const std::string& id : { "Exit", "Settings", "Credits", "End" })
    {
      auto menu = get<C::Menu>(id + ":menu");
      menu->apply([&](C::Image_handle img)
      {
        std::string id = img->entity();
        if (id.find ("_button") != std::string::npos
            || request<C::String>(id + ":text"))
          remove(img->id(), true);
      });
    }

    // Reinit interface
    init();
    create_menu ("Settings");
  }
  else if (setting == "Fullscreen")
  {
    get<C::Boolean>("Window:fullscreen")->set(value == "Yes");
    emit ("Window:toggle_fullscreen");
  }
  else if (setting == "Text_size")
  {
    if (value == "Small")
      set<C::Int>("Dialog:size")->set(Config::SMALL);
    else if (value == "Medium")
      set<C::Int>("Dialog:size")->set(Config::MEDIUM);
    else
      set<C::Int>("Dialog:size")->set(Config::LARGE);
  }
  else if (setting == "Text_speed")
  {
    if (value == "Slow")
      set<C::Int>("Dialog:speed")->set(Config::SLOW);
    else if (value == "Medium_speed")
      set<C::Int>("Dialog:speed")->set(Config::MEDIUM_SPEED);
    else
      set<C::Int>("Dialog:speed")->set(Config::FAST);
  }
  else if (setting == "Music_volume")
  {
    set<C::Int>("Music:volume")->set(to_int(value) / 10);
    emit("Music:volume_changed");
  }
  else if (setting == "Sound_volume")
    set<C::Int>("Sounds:volume")->set(to_int(value) / 10);
}

}
