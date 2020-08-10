/*
  [include/Sosage/Engine.h]
  Inits all systems, holds content and runs main loop.

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

#ifndef SOSAGE_ENGINE_H
#define SOSAGE_ENGINE_H

#define SOSAGE_ASSERTIONS_AS_EXCEPTIONS
#include <Sosage/Content.h>
#include <Sosage/System/Animation.h>
#include <Sosage/System/File_IO.h>
#include <Sosage/System/Graphic.h>
#include <Sosage/System/Input.h>
#include <Sosage/System/Interface.h>
#include <Sosage/System/Logic.h>
#include <Sosage/System/Sound.h>
#include <Sosage/System/Time.h>
#include <Sosage/Utils/error.h>

namespace Sosage
{

class Engine
{
  Content m_content;
  System::Animation m_animation;
  System::Graphic m_graphic;
  System::Sound m_sound;
  System::Input m_input;
  System::Interface m_interface;
  System::File_IO m_file_io;
  System::Logic m_logic;
  System::Time m_time;
public:

  Engine (const std::string& game_name);
  ~Engine();
  void run ();
  int run (const std::string& folder_name);
  void init_interface ();
};

} // namespace Sosage

#endif // SOSAGE_ENGINE_H
