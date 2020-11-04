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
#include <Sosage/System/Handle.h>

#include <set>

namespace Sosage::System
{

class Logic : public Base
{
private:

  typedef std::pair<double, Component::Handle> Timed_handle;
  std::set<Timed_handle> m_timed;

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

  bool apply_step (Component::Action::Step step);
  void action_comment (Component::Action::Step step);
  void action_dialog (Component::Action::Step step);
  void action_fade (Component::Action::Step step, bool fadein);
  void action_fadeout (Component::Action::Step step);
  void action_goto (Component::Action::Step step);
  void action_load (Component::Action::Step step);
  void action_look (const std::string& target);
  void action_modify (const std::string& id, int diff);
  void action_move (Component::Action::Step step);
  void action_play (Component::Action::Step step);
  void action_animate (Component::Action::Step step);
  void action_set_state (Component::Action::Step step);
  void action_set_coordinates (Component::Action::Step step);
  void action_shake (Component::Action::Step step);
  void action_show (Component::Action::Step step);
  void action_show (const std::string& target, bool on);
  void action_sound (const std::string& target);
  void action_start_music (const std::string& target);
  void action_stop_music();
  void action_trigger (const std::string& target);
  void action_wait (double time);

  void create_dialog (const std::string& character,
                      const std::string& text,
                      std::vector<Component::Image_handle>& dialog);

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
