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
#include <Sosage/Content.h>
#include <Sosage/System/Handle.h>

#include <string>
#include <vector>

namespace Sosage::System
{

class Interface : public Base
{

  Content& m_content;
  Config::Layout m_layout;
  Component::Image_handle m_collision;

  int m_action_height;
  double m_verb_scale;

  std::vector<Component::Image_handle> m_verbs;

public:

  Interface (Content& content);

  virtual void run();

  virtual void init();

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
