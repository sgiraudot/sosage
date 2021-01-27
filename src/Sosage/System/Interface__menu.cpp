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

namespace Sosage::System
{

namespace C = Component;

void Interface::init_menus()
{
  auto exit_menu = set<C::Menu>("Exit:menu");

  if constexpr (Config::emscripten)
    exit_menu->split(VERTICALLY, 6);
  else
    exit_menu->split(VERTICALLY, 7);

  init_menu_item ((*exit_menu)[0], "Menu_logo", "");
  init_menu_item ((*exit_menu)[1], "Continue", "cancel");
  init_menu_item ((*exit_menu)[2], "New_game", "new_game");
  init_menu_item ((*exit_menu)[3], "Hint", "hint");
  init_menu_item ((*exit_menu)[4], "Settings", "settings");
  init_menu_item ((*exit_menu)[5], "Credits", "credits");
  if constexpr (!Config::emscripten)
    init_menu_item ((*exit_menu)[6], "Save_and_quit", "quit");
  init_menu_buttons (exit_menu->root());

  auto wanna_restart_menu = set<C::Menu>("Wanna_restart:menu");
  wanna_restart_menu->split(VERTICALLY, 2);
  (*wanna_restart_menu)[1].split(HORIZONTALLY, 2);
  init_menu_item ((*wanna_restart_menu)[0], "Wanna_restart", "");
  init_menu_item ((*wanna_restart_menu)[1][0], "Ok", "ok");
  init_menu_item ((*wanna_restart_menu)[1][1], "Cancel", "cancel");
  init_menu_buttons ((*wanna_restart_menu)[1]);

#if 1
  std::vector<std::array<std::string, 2> > settings_list
      = {
#if !defined(SOSAGE_ANDROID) && !defined(SOSAGE_EMSCRIPTEN)
          { "Fullscreen", "fullscreen" },
#endif
#ifdef SOSAGE_ANDROID
          { "Virtual_cursor", "virtual_cursor" },
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
#endif

  auto cursor_menu = set<C::Menu>("Cursor:menu");
  cursor_menu->split(VERTICALLY, 3);
  (*cursor_menu)[2].split(HORIZONTALLY, 2);
  (*cursor_menu)[2][0].split(VERTICALLY, 2);
  (*cursor_menu)[2][1].split(VERTICALLY, 2);
  init_menu_item ((*cursor_menu)[0], "Cursor_choice", "");
  init_menu_item ((*cursor_menu)[1], "Cursor_choice_later", "");
  init_menu_item ((*cursor_menu)[2][0][0], "Cursor_choice_virtual", "cursor_choice_virtual");
  init_menu_item ((*cursor_menu)[2][1][0], "Cursor_choice_no", "cursor_choice_no");
  init_menu_item ((*cursor_menu)[2][0][1], "Cursor_choice_virtual_text", "");
  init_menu_item ((*cursor_menu)[2][1][1], "Cursor_choice_no_text", "");
  init_menu_buttons ((*cursor_menu)[2][0]);
  init_menu_buttons ((*cursor_menu)[2][1]);

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
}

void Interface::init_menu_item (Component::Menu::Node node, const std::string& id,
                               const std::string& effect)
{
  const std::string& menu_color = get<C::String>("Menu:color")->value();
  auto interface_font = get<C::Font>("Interface:font");
  auto menu_font = get<C::Font>("Menu:font");

  auto text = request<C::String>(id + ":text");

  if (effect == "")
  {
    if (text)
    {
      const std::string& str = text->value();
      std::size_t pos = str.find('\n');
      if (pos == std::string::npos)
      {
        auto img = set<C::Image>(text->entity() + ":image",
                                 menu_font, "000000", text->value());
        img->z() = Config::menu_text_depth;
        img->on() = false;
        img->set_scale(0.75);
        img->set_relative_origin(0.5, 0.5);
        auto pos = set<C::Position>(text->entity() + ":position", Point(0,0));
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
                                   menu_font, "000000", lines[i]);
          img->z() = Config::menu_text_depth;
          img->on() = false;
          img->set_scale(scale);
          img->set_relative_origin(0.5, 0.5);
          auto pos = set<C::Position>(text->entity() + "_" + std::to_string(i)
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
      auto pos = set<C::Position>(id + ":position", Point(0,0));
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
                          interface_font, menu_color, text->value());
      img->z() = Config::menu_text_depth;
      img->on() = false;
      img->set_scale(0.75);
      img->set_relative_origin(0.5, 0.5);
      pos = set<C::Position>(text->entity() + ":position", Point(0,0));
    }
    else
      pos = get<C::Position>(text->entity() + ":position");
    node[0].init(img, pos);

    set<C::String>(text->entity() + ":effect", effect);
  }
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
  auto left_pos = set<C::Position>("Menu_" + effect + "_left_arrow:position", Point(0, 0));
  node_left.init (left_arrow, left_pos);
  auto right_arrow = set<C::Image>("Menu_" + effect + "_right_arrow:image",
                                  get<C::Image>("Menu_right_arrow:image"));
  right_arrow->z() = Config::menu_text_depth;
  right_arrow->set_relative_origin(0.5, 0.5);
  right_arrow->set_collision(BOX);
  auto right_pos = set<C::Position>("Menu_" + effect + "_right_arrow:position", Point(0, 0));
  node_right.init (right_arrow, right_pos);

  std::vector<std::string> possible_values;
  if (effect == "fullscreen")
    possible_values = { "Yes", "No" };
  else if (effect == "virtual_cursor")
    possible_values = { "Yes", "No" };
  else if (effect == "text_size")
    possible_values = { "Small", "Medium", "Large" };
  else if (effect == "text_speed")
    possible_values = { "Slow", "Medium_speed", "Fast" };
  else if (effect == "music_volume" || effect == "sound_volume")
    possible_values = { "0", "10", "20", "30", "40",
                        "50", "60", "70", "80", "90",
                        "100" };

  auto menu_font = get<C::Font>("Menu:font");

  auto pos = set<C::Position>(effect + ":position", Point(0,0));
  for (std::size_t i = 0; i < possible_values.size(); ++ i)
  {
    std::string id = effect + '_' + possible_values[i];
    std::string text;
    if (auto t = request<C::String>(possible_values[i] + ":text"))
      text = t->value();
    else
      text = possible_values[i] + " %";

    auto img = set<C::Image>(id + ":image",
                             menu_font, "000000", text);
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(0.75);
    img->set_relative_origin(0.5, 0.5);
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
          pos = set<C::Position>(img->entity() + ":position", Point(0,0));
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
          pos = set<C::Position>(img->entity() + ":position", Point(0,0));
        }
        else
          pos = get<C::Position>(img->entity() + ":position");
        node[i].init(img, pos);
      }
  }
}

void Interface::update_exit()
{
  auto status = get<C::Status>(GAME__STATUS);
  if (status->value() == LOCKED)
  {
    receive("Game:escape");
    return;
  }
  if (receive("Show:menu"))
  {
    create_menu (get<C::String>("Game:triggered_menu")->value());
    status->push (IN_MENU);
  }

  if (status->value() == CUTSCENE)
  {
    double time = get<C::Double>(CLOCK__TIME)->value();
    bool exit_message_exists = (request<C::Image>("Exit_message:image") != nullptr);

    if (receive("Game:escape"))
    {
      if (time - m_latest_exit < Config::key_repeat_delay)
      {
        emit("Game:skip_cutscene");
        if (exit_message_exists)
        {
          remove("Exit_message:image");
          remove("Exit_message:position");
          remove("Exit_message_back:image");
          remove("Exit_message_back:position");
        }
        return;
      }
      m_latest_exit = time;
    }


    if (time - m_latest_exit < Config::key_repeat_delay)
    {
      if (!exit_message_exists)
      {
        auto interface_font = get<C::Font> ("Interface:font");

        auto img
            = set<C::Image>("Exit_message:image", interface_font, "FFFFFF",
                            get<C::String>("Skip_cutscene:text")->value());
        img->z() += 10;
        img->set_scale(0.5);
        img->set_relative_origin (1, 1);

        auto img_back
            = set<C::Image>("Exit_message_back:image", 0.5 * img->width() + 10, 0.5 * img->height() + 10);
        img_back->z() = img->z() - 1;
        img_back->set_relative_origin (1, 1);

        int window_width = Config::world_width;
        int window_height = Config::world_height;
        set<C::Position>("Exit_message:position", Point (window_width - 5,
                                                         window_height - 5));
        set<C::Position>("Exit_message_back:position", Point (window_width,
                                                              window_height));
      }
    }
    else
    {
      if (exit_message_exists)
      {
        remove("Exit_message:image");
        remove("Exit_message:position");
        remove("Exit_message_back:image");
        remove("Exit_message_back:position");
      }
    }
  }
  else // status != CUTSCENE
  {
    if (receive("Game:escape"))
    {
      if (status->value() == IN_MENU)
      {
        const std::string& menu = get<C::String>("Game:current_menu")->value();
        if (menu == "End")
          emit("Game:exit");
        else
        {
          delete_menu(menu);
          status->pop();
        }
      }
      else
      {
        create_menu("Exit");
        status->push (IN_MENU);
      }
    }
  }

}

void Interface::
create_menu (const std::string& id)
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
    menu->update_setting ("fullscreen",
                          get<C::Boolean>("Window:fullscreen")->value() ? "Yes" : "No");

    menu->update_setting ("virtual_cursor",
                          get<C::Boolean>("Interface:virtual_cursor")->value() ? "Yes" : "No");

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

  const std::string& menu_color = get<C::String>("Menu:color")->value();

  Vector size = menu->size();

  auto color = color_from_string (menu_color);
  auto img = set<C::Image>("Menu_foreground:image", size.x(), size.y(),
                           color[0], color[1], color[2]);
  img->set_relative_origin (0.5, 0.5);
  img->z() = Config::menu_front_depth;
  set<C::Position>("Menu_foreground:position",
                   Point (Config::world_width / 2,
                          Config::world_height / 2));

  auto img2 = set<C::Image>("Menu_background:image", size.x() + 10, size.y() + 10);

  img2->set_relative_origin (0.5, 0.5);
  img2->z() = Config::menu_back_depth;
  set<C::Position>("Menu_background:position",
                   Point (Config::world_width / 2,
                          Config::world_height / 2));


  menu->set_position(Config::world_width / 2,
                     Config::world_height / 2);

}

void Interface::delete_menu (const std::string& id)
{
  auto menu = get<C::Menu>(id + ":menu");
  menu->hide();
  get<C::Image>("Menu_background:image")->on() = false;
  get<C::Image>("Menu_foreground:image")->on() = false;
}

void Interface::menu_clicked()
{
  std::string entity = m_collision->entity();
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
      get<C::Status>(GAME__STATUS)->pop();
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
      get<C::Status>(GAME__STATUS)->pop();
    }
    else if (menu == "Credits" || menu == "Settings")
    {
      delete_menu (menu);
      create_menu ("Exit");
    }
    else if (menu == "Hint")
    {
      delete_menu("Hint");
      get<C::Status>(GAME__STATUS)->pop();
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
      get<C::Status>(GAME__STATUS)->pop();
    else if (menu == "Wanna_restart")
      create_menu("Exit");
  }
  else if (effect->value() == "cursor_choice_virtual")
  {
    get<C::Boolean>("Interface:virtual_cursor")->set (true);
    delete_menu("Cursor");
    get<C::Status>(GAME__STATUS)->pop();
  }
  else if (effect->value() == "cursor_choice_no")
  {
    get<C::Boolean>("Interface:virtual_cursor")->set (false);
    delete_menu("Cursor");
    get<C::Status>(GAME__STATUS)->pop();
  }
}

void Interface::apply_setting (const std::string& setting, const std::string& value)
{
  std::cerr << value << std::endl;
  if (setting == "fullscreen")
  {
    get<C::Boolean>("Window:fullscreen")->set(value == "Yes");
    emit ("Window:toggle_fullscreen");
  }
  else if (setting == "virtual_cursor")
    get<C::Boolean>("Interface:virtual_cursor")->set(value == "Yes");
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
