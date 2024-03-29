/*
  [include/Sosage/System/Test_input.h]
  Automatic input generation for testing purposes.

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

#ifndef SOSAGE_SYSTEM_TEST_INPUT_H
#define SOSAGE_SYSTEM_TEST_INPUT_H

#ifdef SOSAGE_DEV

#include <Sosage/Core/Input.h>
#include <Sosage/System/Base.h>
#include <Sosage/Utils/geometry.h>

#include <random>

namespace Sosage::System
{

class Test_input : public Base
{
  Core::Input m_core;
  bool m_random_mode;
  std::function<void(void)> m_mode;
  std::mt19937 m_randgen;

public:

  Test_input (Content& content);
  void set_random_mode();

  virtual void run();

private:

  void run_mouse();
  void run_mouse_chaos();
  void run_touchscreen();
  void run_gamepad();
  void run_gamepad_chaos();

  Point cursor_target (const std::string& id);

  std::function<void(void)> new_mode();
  bool ready (const std::string& key, double time);
};

} // namespace Sosage::System

#endif // SOSAGE_DEV

#endif // SOSAGE_SYSTEM_TEST_INPUT_H
