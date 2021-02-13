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
  Component::Image_handle m_collision;
  std::vector<std::string> m_action_ids;
  double m_latest_exit;

public:

  Interface (Content& content);

  virtual void run();

  virtual void init();

private:

  void window_clicked();
  void code_clicked (Component::Position_handle cursor);
  void dialog_clicked ();
  void verb_clicked();
  void arrow_clicked();
  void action_clicked();

  void update_pause_screen();

  void detect_collision (Component::Position_handle cursor);
  void clear_action_ids();
  void update_label (const std::string& id, std::string name, bool open_left,
                     bool open_right, const Point& position, const Collision_type& collision);
  void generate_action_choice (const std::string& id);
  void generate_action (const std::string& id, const std::string& action, const Config::Orientation& orientation);
  void update_inventory ();
  void update_dialog_choices ();

  // Implemented in Interface__menu.cpp
  void init_menus();
  void update_exit();
  void init_menu_item (Component::Menu::Node node, const std::string& id,
                       const std::string& effect = std::string());
  void init_setting_item (Component::Menu::Node node_left,
                          Component::Menu::Node node,
                          Component::Menu::Node node_right,
                          const std::string& effect);
  void init_menu_buttons (Component::Menu::Node node);
  void create_menu (const std::string& id);
  void delete_menu (const std::string& id);
  void menu_clicked ();
  void apply_setting (const std::string& setting, const std::string& value);

};


} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INTERFACE_H
