/*
  [include/Sosage/System/Interface.h]
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

#ifndef SOSAGE_SYSTEM_INTERFACE_H
#define SOSAGE_SYSTEM_INTERFACE_H

#include <Sosage/Component/Group.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Menu.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Content.h>
#include <Sosage/System/Handle.h>

#include <string>
#include <vector>

namespace Sosage::System
{

class Interface : public Base
{
  std::string m_active_object;
  std::vector<std::string> m_active_objects;
  std::array<std::string, 4> m_action_selector;

  enum Animation_style { NONE, DEPLOY, FADE, FADE_LABEL_ONLY, ZOOM };

public:

  Interface (Content& content);

  virtual void run();

  virtual void init();

private:

  void update_active_objects();
  void update_action_selector();
  void update_object_switcher();
  void update_inventory();
  void update_code_hover();
  void update_dialog_choices();
  void update_skip_message();
  void update_cursor();

  // Implemented in Interface__labels.cpp
  void create_object_label (const std::string& id);
  void create_label (bool is_button, const std::string& id, std::string name,
                     bool open_left, bool open_right,
                     const Collision_type& collision, double scale = 1.0, bool arrow = false);
  void animate_label (const std::string& id, const Animation_style& style,
                      bool button = false, const Point& position = Point());
  void update_label (const std::string& id,
                     bool open_left, bool open_right, Component::Position_handle pos,
                     double scale = 1.0, bool arrow = false);
  void update_label_position (const std::string& id, double scale = 1.0);
  void delete_label (const std::string& id);
  void fade_action_selector (const std::string& id, bool fade_in);
  void highlight_object (const std::string& id, unsigned char highlight);
  void set_action_selector (const std::string& id);
  void reset_action_selector ();
  void generate_action (const std::string& id, const std::string& action,
                        const Button_orientation& orientation, const std::string& button,
                        Component::Position_handle position, const Animation_style& style = NONE);

  // Implemented in Interface__menu.cpp
  void init_menus();
  void update_menu();
  void update_exit();

  void make_oknotok_item (Component::Menu::Node node, bool only_ok);
  void make_exit_menu_item (Component::Menu::Node node, const std::string& id, int y);
  void make_text_menu_title (Component::Menu::Node node, const std::string& id);
  void make_text_menu_text (Component::Menu::Node node, const std::string& id);
  void make_settings_item (Component::Menu::Node node, const std::string& id, int y);

  void create_menu (const std::string& id);
  void delete_menu (const std::string& id);
  void menu_clicked ();
  void apply_setting (const std::string& setting, const std::string& value);
};


} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INTERFACE_H
