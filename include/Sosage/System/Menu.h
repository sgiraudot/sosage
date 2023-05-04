/*
  [include/Sosage/System/Menu.h]
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

#ifndef SOSAGE_SYSTEM_MENU_H
#define SOSAGE_SYSTEM_MENU_H

#include <Sosage/Component/Menu.h>
#include <Sosage/Content.h>
#include <Sosage/System/Base.h>

namespace Sosage
{

namespace Config
{
constexpr auto exit_menu_items
= {  "Phone", "Settings", "Credits", "New_game"
     #ifndef SOSAGE_EMSCRIPTEN
     , "Load", "Save", "Quit"
     #endif
  };
constexpr auto menus
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
} // namespace Config

namespace System
{

class Menu : public Base
{
private:

  using Button_id = Component::Id;
  using Callback = std::function<void(const std::string& menu,
                                      const std::string& effect)>;
  std::unordered_map<Button_id, Callback, Component::Id_hash> m_callbacks;

public:

  Menu (Content& content);

  virtual void run();

  virtual void init();

private:

  void update_menu();
  void update_exit();
  void show_menu (const std::string& id);
  void hide_menu (const std::string& id);
  void delete_menu (const std::string& id);
  void menu_clicked ();
  void apply_setting (const std::string& setting, const std::string& value);
  void update_phone_menu();

  // Implemented in Menu__creation.cpp
  void init_menus();
  void init_loadsave_menus();
  void create_callback (const std::string& menu, const std::string& button,
                        const Callback& callback);
  void make_oknotok_item (Component::Menu::Node node, bool only_ok);
  void make_exit_menu_item (Component::Menu::Node node, const std::string& id, int y);
  void make_text_menu_title (Component::Menu::Node node, const std::string& id);
  void make_text_menu_text (Component::Menu::Node node, const std::string& id, bool credits = false);
  void make_settings_item (Component::Menu::Node node, const std::string& id, int y);

  std::pair<Component::Image_handle, Component::Position_handle>
  make_settings_button (const std::string& id, int y);
  std::pair<Component::Image_handle, Component::Position_handle>
  make_settings_title (const std::string& id, const std::string& suffix, int y,
                       const std::string& text);
  std::pair<Component::Image_handle, Component::Position_handle>
  make_settings_subtitle (const std::string& id, const std::string& suffix, int y,
                          const std::string& text);
};

} // namespace System

} // namespace Sosage

#endif // SOSAGE_SYSTEM_MENU_H
