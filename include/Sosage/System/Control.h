/*
  [include/Sosage/System/Control.h]
  Handles how users control the interface.

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

#ifndef SOSAGE_SYSTEM_CONTROL_H
#define SOSAGE_SYSTEM_CONTROL_H

#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Handle.h>

namespace Sosage::System
{

class Control : public Base
{
  Status m_status;
  Input_mode m_mode;
  bool m_stick_on;

  using Function = std::function<void(void)>;
  using Status_input_pair = std::pair<Status, Input_mode>;

  struct Hash_status_mode_pair
  {
    std::size_t operator() (const Status_input_pair& p) const
    {
      return std::hash<std::size_t>()(std::size_t(p.first))
          ^  std::hash<std::size_t>()(std::size_t(p.second));
    }
  };

  std::unordered_map<Status_input_pair, Function, Hash_status_mode_pair> m_dispatcher;

public:

  Control (Content& content);

  virtual void run();

  virtual void init() { }

private:

  void begin_status (const Status& s);
  void end_status (const Status& s);

  void idle_mouse();
  void idle_touchscreen();
  void idle_gamepad();

  void idle_sub_click (const std::string& target);
  void idle_sub_update_active_objects();
  void idle_sub_switch_active_object (bool right);
  void idle_sub_triggered (const std::string& key);

  void action_choice_mouse();
  void action_choice_touchscreen();

  void action_choice_sub_click (const std::string& id);

  void object_choice_mouse();
  void object_choice_touchscreen();
  void object_choice_gamepad();

  void object_choice_sub_click (const std::string& id);
  void object_choice_sub_triggered (const std::string& key);

  void inventory_mouse();
  void inventory_touchscreen();
  void inventory_gamepad();

  void inventory_sub_click (const std::string& target);
  void inventory_sub_switch_active_object (bool right);
  void inventory_sub_triggered (const std::string& key);

  void window_mouse();
  void window_touchscreen();
  void window_gamepad();

  void code_mouse();
  void code_touchscreen();
  void code_gamepad();

  void code_sub_click(bool collision);

  bool collides (Component::Position_handle cursor, Component::Image_handle img);
  std::string first_collision (Component::Position_handle cursor,
                               const std::function<bool(Component::Image_handle)>& filter);
  std::vector<std::string> detect_active_objects();
  void set_action (const std::string& id, const std::string& default_id);
  Event_value stick_left_right();
  Event_value stick_left_right_up_down();
  Vector stick_direction();

};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_CONTROL_H
