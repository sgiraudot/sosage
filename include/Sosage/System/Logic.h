/*
  [include/Sosage/System/Interface.h]
  Game logic, actions, dialog generation, etc.

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

#ifndef SOSAGE_SYSTEM_LOGIC_H
#define SOSAGE_SYSTEM_LOGIC_H

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Content.h>
#include <Sosage/System/Handle.h>

#include <set>

namespace Sosage::System
{

class Logic : public Base
{
private:

  typedef std::pair<double, Component::Handle> Timed_handle;
  std::set<Timed_handle> m_timed;

  Content& m_content;
  double m_current_time;

  Component::Action_handle m_current_action;
  std::size_t m_next_step;

public:

  Logic (Content& content);

  virtual void run();

private:
  void clear_timed(bool action_goto);

  bool compute_path_from_target (Component::Position_handle target);
  void update_camera();
  void update_debug_info (Component::Debug_handle debug_info);

  void action_comment (Component::Action::Step step);
  void action_goto (const std::string& target);
  void action_load (Component::Action::Step step);
  void action_look (const std::string& target);
  void action_move (Component::Action::Step step);
  void action_play (Component::Action::Step step);
  void action_pick_animation (Component::Action::Step step);
  void action_set_state (Component::Action::Step step);
  void action_set_coordinates (Component::Action::Step step);
  void action_show (Component::Action::Step step);

  void create_dialog (const std::string& text, std::vector<Component::Image_handle>& dialog);
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
