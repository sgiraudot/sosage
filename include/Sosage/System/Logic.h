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
#include <Sosage/System/Base.h>

namespace Sosage::System
{

class Logic : public Base
{
private:

  double m_current_time;
  using Function = std::function<bool(const std::vector<std::string>&)>;
  std::unordered_map<std::string, Function> m_dispatcher;
  Component::Action_handle m_current_action;

public:

  Logic (Content& content);

  virtual void run();

private:

  void run_cutscene();

  bool compute_path_from_target (Component::Position_handle target,
                                 std::string id = "");
  bool compute_path_from_direction (const Vector& direction);
  void follow (const std::string& follower);
  void update_debug_info (Component::Debug_handle debug_info);

  bool apply_next_step (Component::Action_handle action);

  bool function_add (const std::vector<std::string>& args);
  bool function_camera (const std::vector<std::string>& args);
  bool function_dialog (const std::vector<std::string>& args);
  bool function_goto (const std::vector<std::string>& args);
  bool function_look (const std::vector<std::string>& args);
  bool function_play (const std::vector<std::string>& args);
  bool function_set (const std::vector<std::string>& args);
  bool function_stop (const std::vector<std::string>& args);
  bool function_system (const std::vector<std::string>& args);
  bool function_talk (const std::vector<std::string>& args);


  void create_dialog (const std::string& character,
                      const std::string& text,
                      std::vector<Component::Image_handle>& dialog);
  void create_hints();

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
