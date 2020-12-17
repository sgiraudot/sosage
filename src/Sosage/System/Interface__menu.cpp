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
  auto exit_menu = set<C::Menu>("exit:menu");
  exit_menu->split(VERTICALLY, 7);
  init_menu_item ((*exit_menu)[0], "menu_logo", "");
  init_menu_item ((*exit_menu)[1], "continue", "cancel");
  init_menu_item ((*exit_menu)[2], "new_game", "new_game");
  init_menu_item ((*exit_menu)[3], "hint", "hint");
  init_menu_item ((*exit_menu)[4], "settings", "settings");
  init_menu_item ((*exit_menu)[5], "credits", "credits");
  init_menu_item ((*exit_menu)[6], "quit", "quit");
  init_menu_buttons (exit_menu->root());

  auto wanna_restart_menu = set<C::Menu>("wanna_restart:menu");
  wanna_restart_menu->split(VERTICALLY, 2);
  (*wanna_restart_menu)[1].split(HORIZONTALLY, 2);
  init_menu_item ((*wanna_restart_menu)[0], "wanna_restart", "");
  init_menu_item ((*wanna_restart_menu)[1][0], "ok", "ok");
  init_menu_item ((*wanna_restart_menu)[1][1], "cancel", "cancel");
  init_menu_buttons ((*wanna_restart_menu)[1]);

  auto credits_menu = set<C::Menu>("credits:menu");
  credits_menu->split(VERTICALLY, 3);
  init_menu_item ((*credits_menu)[0], "credits_logo", "");
  init_menu_item ((*credits_menu)[1], "credits_text", "");
  init_menu_item ((*credits_menu)[2], "ok", "ok");
  init_menu_buttons (credits_menu->root());
}

void Interface::init_menu_item (Component::Menu::Node node, const std::string& id,
                               const std::string& effect)
{
  const std::string& menu_color = get<C::String>("menu:color")->value();
  auto interface_font = get<C::Font>("interface:font");
  auto menu_font = get<C::Font>("menu:font");

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

        node.split(VERTICALLY, lines.size());
        for (std::size_t i = 0; i < lines.size(); ++ i)
        {
          auto img = set<C::Image>(text->entity() + "_" + std::to_string(i)
                                   + ":image",
                                   menu_font, "000000", lines[i]);
          img->z() = Config::menu_text_depth;
          img->on() = false;
          img->set_scale(0.75);
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
    auto img = set<C::Image>(text->entity() + ":image",
                             interface_font, menu_color, text->value());
    img->z() = Config::menu_text_depth;
    img->on() = false;
    img->set_scale(0.75);
    img->set_relative_origin(0.5, 0.5);
    auto pos = set<C::Position>(text->entity() + ":position", Point(0,0));
    node[0].init(img, pos);

    set<C::String>(text->entity() + ":effect", effect);
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
        auto img = set<C::Image>(node[i][0].image()->entity() + "_button:image",
            width, node[i][0].size().y());
        img->z() = Config::menu_button_depth;
        img->set_relative_origin(0.5, 0.5);
        img->on() = false;
        auto pos = set<C::Position>(img->entity() + ":position", Point(0,0));
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
        auto img = set<C::Image>(node[i][0].image()->entity() + "_button:image",
            node[i][0].size().x(), height);
        img->z() = Config::menu_button_depth;
        img->set_relative_origin(0.5, 0.5);
        img->on() = false;
        auto pos = set<C::Position>(img->entity() + ":position", Point(0,0));
        node[i].init(img, pos);
      }
  }
}

void Interface::update_exit()
{
  auto status = get<C::Status>(GAME__STATUS);
  if (status->value() == LOCKED)
  {
    receive("game:escape");
    return;
  }

  if (status->value() == CUTSCENE)
  {
    double time = get<C::Double>(CLOCK__TIME)->value();
    bool exit_message_exists = (request<C::Image>("exit_message:image") != nullptr);

    if (receive("game:escape"))
    {
      if (time - m_latest_exit < Config::key_repeat_delay)
      {
        emit("game:skip_cutscene");
        if (exit_message_exists)
        {
          remove("exit_message:image");
          remove("exit_message:position");
        }
        return;
      }
      m_latest_exit = time;
    }


    if (time - m_latest_exit < Config::key_repeat_delay)
    {
      if (!exit_message_exists)
      {
        auto interface_font = get<C::Font> ("interface:font");

        auto img
            = set<C::Image>("exit_message:image", interface_font, "FFFFFF",
                            get<C::String>("skip_cutscene:text")->value());
        img->z() += 10;
        img->set_scale(0.5);

        set<C::Position>("exit_message:position", Point(5,5));
      }
    }
    else
    {
      if (exit_message_exists)
      {
        remove("exit_message:image");
        remove("exit_message:position");
      }
    }
  }
  else // status != CUTSCENE
  {
    if (receive("game:escape"))
    {
      if (status->value() == IN_MENU)
      {
        delete_menu(get<C::String>("game:current_menu")->value());
        status->pop();
      }
      else
      {
        create_menu("exit");
        status->push (IN_MENU);
      }
    }
  }

}

void Interface::create_menu (const std::string& id)
{
  set<C::String>("game:current_menu", id);

  // Hint menu is created on the fly
  if (id == "hint")
  {
    std::string hint = get<C::Hints>("game:hints")->next();
    if (hint == "")
      hint = get<C::String>("no_hint:text")->value();
    set<C::String>("hint_text:text", hint);
    auto hint_menu = set<C::Menu>("hint:menu");
    hint_menu->split(VERTICALLY, 2);
    init_menu_item ((*hint_menu)[0], "hint_text", "");
    init_menu_item ((*hint_menu)[1], "ok", "ok");
    init_menu_buttons (hint_menu->root());
  }

  auto menu = get<C::Menu>(id + ":menu");
  const std::string& menu_color = get<C::String>("menu:color")->value();

  Vector size = menu->size();

  auto color = color_from_string (menu_color);
  auto img = set<C::Image>("menu_foreground:image", size.x(), size.y(),
                           color[0], color[1], color[2]);
  img->set_relative_origin (0.5, 0.5);
  img->z() = Config::menu_front_depth;
  set<C::Position>("menu_foreground:position",
                   Point (Config::world_width / 2,
                          Config::world_height / 2));

  auto img2 = set<C::Image>("menu_background:image", size.x() + 10, size.y() + 10);

  img2->set_relative_origin (0.5, 0.5);
  img2->z() = Config::menu_back_depth;
  set<C::Position>("menu_background:position",
                   Point (Config::world_width / 2,
                          Config::world_height / 2));


  menu->set_position(Config::world_width / 2,
                     Config::world_height / 2);

}

void Interface::delete_menu (const std::string& id)
{
  auto menu = get<C::Menu>(id + ":menu");
  menu->hide();
  get<C::Image>("menu_background:image")->on() = false;
  get<C::Image>("menu_foreground:image")->on() = false;
}

}
