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
constexpr int exit_menu_margin = 85;
constexpr int exit_menu_small_margin = 35;
constexpr int exit_menu_logo = 130;
constexpr int exit_menu_start = 300;
constexpr int exit_menu_text = exit_menu_margin + 60;
constexpr int exit_menu_button_width = 400;
constexpr int exit_menu_button_height = 80;
constexpr int exit_menu_oknotok_y = 858;
constexpr int exit_menu_oknotok_height = 858;
constexpr int exit_menu_ok_x = 120;
constexpr int exit_menu_notok_x = 360;
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
    y += Config::exit_menu_margin;
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

#if 0
  init_menu_item ((*exit_menu)[0], "Menu_logo", "");
  init_menu_item ((*exit_menu)[1], "New_game", "new_game");
  init_menu_item ((*exit_menu)[2], "Settings", "settings");
  init_menu_item ((*exit_menu)[4], "Phone", "phone");
  init_menu_item ((*exit_menu)[5], "GPS", "gps");
    init_menu_item ((*exit_menu)[6], "Credits", "credits");
  if constexpr (!Config::emscripten)
    init_menu_item ((*exit_menu)[7], "Save_and_quit", "quit");
  init_menu_buttons (exit_menu->root());

  auto wanna_restart_menu = set<C::Menu>("Wanna_restart:menu");
  wanna_restart_menu->split(VERTICALLY, 2);
  (*wanna_restart_menu)[1].split(HORIZONTALLY, 2);
  init_menu_item ((*wanna_restart_menu)[0], "Wanna_restart", "");
  init_menu_item ((*wanna_restart_menu)[1][0], "Ok", "ok");
  init_menu_item ((*wanna_restart_menu)[1][1], "Cancel", "cancel");
  init_menu_buttons ((*wanna_restart_menu)[1]);

  std::vector<std::array<std::string, 2> > settings_list
      = {
          { "Language", "language" },
#if !defined(SOSAGE_ANDROID) && !defined(SOSAGE_EMSCRIPTEN)
          { "Fullscreen", "fullscreen" },
#endif
          { "Text_size", "text_size" },
          { "Text_speed", "text_speed" },
          { "Music_volume", "music_volume" },
          { "Sound_volume", "sound_volume" } };
  auto settings_menu = set<C::Menu>("Settings:menu");
  settings_menu->split(VERTICALLY, 2);
  (*settings_menu)[0].split(HORIZONTALLY, 4);
  for (std::size_t i = 0; i < 4; ++ i)
    (*settings_menu)[0][i].split(VERTICALLY, settings_list.size());
  for (std::size_t i = 0; i < settings_list.size(); ++ i)
  {
    init_menu_item ((*settings_menu)[0][0][i], settings_list[i][0], "");
    init_setting_item ((*settings_menu)[0][1][i], (*settings_menu)[0][2][i],
        (*settings_menu)[0][3][i], settings_list[i][1]);
  }
  init_menu_item ((*settings_menu)[1], "Ok", "ok");
  init_menu_buttons (settings_menu->root());

  auto credits_menu = set<C::Menu>("Credits:menu");
  credits_menu->split(VERTICALLY, 3);
  init_menu_item ((*credits_menu)[0], "Credits_logo", "");
  init_menu_item ((*credits_menu)[1], "Credits_text", "");
  init_menu_item ((*credits_menu)[2], "Ok", "ok");
  init_menu_buttons (credits_menu->root());

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
    pos_icon = set<C::Relative_position>(id + "_icon:position", reference, Vector (Config::exit_menu_margin, y));
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
    pos = get<C::Position>(img->entity() + "_button:position");
  node.init(button, pos_button);

  std::string effect (id);
  for (char& c : effect)
    c = std::tolower(c);

  set<C::String>(text->entity() + ":effect", effect);
}

void Interface::make_oknotok_item (Component::Menu::Node node, bool only_ok)
{
  C::Image_handle ok, cancel, ok_alone, ok_button, cancel_button, ok_alone_button;
  C::Position_handle ok_pos, cancel_pos, ok_alone_pos,
      ok_button_pos, cancel_button_pos, ok_alone_button_pos;
  ok_pos = request<C::Position>("Ok_icon:position");

  // Only create once
  if (!ok_pos)
  {
    auto reference = get<C::Position>("Menu:reference");

    ok = get<C::Image>("Ok_icon:image");
    ok->z() = Config::menu_text_depth;
    ok->on() = false;
    ok->set_relative_origin(0.5, 0.5);
    ok->set_collision(UNCLICKABLE);
    ok_alone = set<C::Image>("Ok_alone_icon:image", ok);

    ok_pos = set<C::Relative_position>("Ok_icon:position", reference,
                                       Vector (Config::exit_menu_ok_x, Config::exit_menu_oknotok_y));
    ok_alone_pos = set<C::Relative_position>("Ok_alone_icon:position", reference,
                                             Vector (240, Config::exit_menu_oknotok_y));

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

    set<C::String>("Ok:effect", "ok");
    set<C::String>("Ok_alone:effect", "ok");

    cancel = get<C::Image>("Cancel_icon:image");
    cancel->z() = Config::menu_text_depth;
    cancel->on() = false;
    cancel->set_relative_origin(0.5, 0.5);
    cancel->set_collision(UNCLICKABLE);
    cancel_pos = set<C::Relative_position>("Cancel_icon:position", reference,
                                           Vector (Config::exit_menu_notok_x, Config::exit_menu_oknotok_y));

    cancel_button = set<C::Image>("Cancel_button:image", 230, 50, 0, 0, 0, 64);
    cancel_button->set_relative_origin(0.5, 0.5);
    cancel_button->on() = false;
    cancel_button_pos = set<C::Relative_position>("Cancel_button:position", cancel_pos, Vector(0,0));
    cancel_button->z() = Config::menu_button_depth;

    set<C::String>("Cancel:effect", "cancel");
  }
  else
  {
    ok = get<C::Image>("Ok_icon:image");
    cancel = get<C::Image>("Cancel_icon:image");
    ok_alone = get<C::Image>("Ok_alone_icon:image");
    ok_button = get<C::Image>("Ok_button:image");
    cancel_button = get<C::Image>("Cancel_button:image");
    ok_alone_button = get<C::Image>("Ok_alone_button:image");
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
                                       Point(240, Config::exit_menu_margin));
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
                                         Point(Config::exit_menu_small_margin, y));
    node[i].init(img, pos);
    y += Config::exit_menu_small_margin;
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

      if (gamepad && settings &&  contains (entity, "arrow"))
        current.image()->on() = active;
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
    menu->update_setting ("language",
                          get<C::String>(get<C::String>(GAME__CURRENT_LOCAL)->value() + ":description")->value());

    menu->update_setting ("fullscreen",
                          get<C::Boolean>("Window:fullscreen")->value() ? "Yes" : "No");

    int speed = get<C::Int>("Dialog:speed")->value();
    if (speed == Config::SLOW)
      menu->update_setting ("text_speed", "Slow");
    else if (speed == Config::MEDIUM_SPEED)
      menu->update_setting ("text_speed", "Medium_speed");
    else if (speed == Config::FAST)
      menu->update_setting ("text_speed", "Fast");

    int size = get<C::Int>("Dialog:size")->value();
    if (size == Config::SMALL)
      menu->update_setting ("text_size", "Small");
    else if (size == Config::MEDIUM)
      menu->update_setting ("text_size", "Medium");
    else if (size == Config::LARGE)
      menu->update_setting ("text_size", "Large");

    menu->update_setting ("music_volume", std::to_string(10 * get<C::Int>("Music:volume")->value()));
    menu->update_setting ("sound_volume", std::to_string(10 * get<C::Int>("Sounds:volume")->value()));
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
  if (!effect)
  {
    if (menu == "Settings")
    {
      std::size_t pos = entity.find("_left_arrow");
      bool left_arrow = (pos != std::string::npos);
      if (!left_arrow)
      {
        pos = entity.find("_right_arrow");
        if (pos == std::string::npos)
          return;
      }

      emit("Click:play_sound");
      std::string setting (entity.begin() + 5, entity.begin() + pos);
      if (left_arrow)
        apply_setting (setting, get<C::Menu>("Settings:menu")->decrement(setting));
      else
        apply_setting (setting, get<C::Menu>("Settings:menu")->increment(setting));
    }
    return;
  }

  emit("Click:play_sound");


  if (effect->value() == "quit")
  {
    if (menu != "End")
      emit ("Game:save");
    emit ("Game:exit");
  }
  else if (effect->value() == "new_game")
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
  else if (effect->value() == "ok")
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
  else if (effect->value() == "settings")
  {
    delete_menu ("Exit");
    create_menu ("Settings");
  }
  else if (effect->value() == "credits")
  {
    delete_menu ("Exit");
    create_menu ("Credits");
  }
  else if (effect->value() == "hint")
  {
    delete_menu ("Exit");
    create_menu ("Hint");
  }
  else if (effect->value() == "cancel")
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
  if (setting == "language")
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
  else if (setting == "fullscreen")
  {
    get<C::Boolean>("Window:fullscreen")->set(value == "Yes");
    emit ("Window:toggle_fullscreen");
  }
  else if (setting == "text_size")
  {
    if (value == "Small")
      set<C::Int>("Dialog:size")->set(Config::SMALL);
    else if (value == "Medium")
      set<C::Int>("Dialog:size")->set(Config::MEDIUM);
    else
      set<C::Int>("Dialog:size")->set(Config::LARGE);
  }
  else if (setting == "text_speed")
  {
    if (value == "Slow")
      set<C::Int>("Dialog:speed")->set(Config::SLOW);
    else if (value == "Medium_speed")
      set<C::Int>("Dialog:speed")->set(Config::MEDIUM_SPEED);
    else
      set<C::Int>("Dialog:speed")->set(Config::FAST);
  }
  else if (setting == "music_volume")
  {
    set<C::Int>("Music:volume")->set(to_int(value) / 10);
    emit("Music:volume_changed");
  }
  else if (setting == "sound_volume")
    set<C::Int>("Sounds:volume")->set(to_int(value) / 10);
}

}
