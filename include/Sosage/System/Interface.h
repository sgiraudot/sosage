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
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/Content.h>

#include <string>
#include <vector>

namespace Sosage::System
{

class Interface
{
  enum Layout { INIT, WIDESCREEN, STANDARD, SQUARE, PORTRAIT };

  Content& m_content;
  bool m_auto_layout;
  Layout m_layout;
  Component::Image_handle m_collision;
  
  const int m_action_min_height;
  const int m_interface_min_height;

  int m_action_height;
  double m_verb_scale;

  std::vector<Component::Image_handle> m_verbs;

public:

  Interface (Content& content);

  void run();

  void init();

private:
  void update_responsive();
  void interface_widescreen();
  void interface_standard();
  void interface_square();
  void interface_portrait();

  void vertical_layout();
  void horizontal_layout();

  void window_clicked();
  void code_clicked (Component::Position_handle cursor);
  void verb_clicked();
  void arrow_clicked();
  void action_clicked(const std::string& verb);

  void update_pause_screen();
  
  void detect_collision (Component::Position_handle cursor);
  void update_action ();
  void update_inventory ();

};


} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INTERFACE_H
