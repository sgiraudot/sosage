/*
  [src/Sosage/Engine.cpp]
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

#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Config/platform.h>
#include <Sosage/Config/version.h>
#include <Sosage/Engine.h>
#include <Sosage/System/Animation.h>
#include <Sosage/System/File_IO.h>
#include <Sosage/System/Graphic.h>
#include <Sosage/System/Input.h>
#include <Sosage/System/Interface.h>
#include <Sosage/System/Logic.h>
#include <Sosage/System/Sound.h>
#include <Sosage/System/Time.h>
#include <Sosage/Utils/profiling.h>
#include <Sosage/Utils/file.h>

#include <ctime>

namespace Sosage
{

Engine::Engine ()
{
  debug ("Running Sosage " + Sosage::Version::str());
  srand(time(nullptr));
}

Engine::~Engine()
{
  // Clear content before shutting down systems
  m_content.clear();
}

int Engine::run (const std::string& folder_name)
{
  // Init main variables
  m_content.set_fac<Component::Status>(GAME__STATUS, "game:status");
  m_content.set_fac<Component::Double>(CAMERA__POSITION, "camera:position", 0.0);
  m_content.set<Component::Double>("camera:target", 0.0);
  m_content.set<Component::Inventory>("game:inventory");

  // Raise exception now if folder does not exit
  std::shared_ptr<System::File_IO>
    file_io = System::make_handle<System::File_IO>(m_content);
  file_io->test_init_folder (folder_name);

  // Create all systems
  m_systems.push_back (file_io);
  m_systems.push_back (System::make_handle<System::Graphic>(m_content));
  m_systems.push_back (System::make_handle<System::Input>(m_content));
  m_systems.push_back (System::make_handle<System::Logic>(m_content));
  m_systems.push_back (System::make_handle<System::Interface>(m_content));
  m_systems.push_back (System::make_handle<System::Animation>(m_content));
  m_systems.push_back (System::make_handle<System::Sound>(m_content));
  m_systems.push_back (System::make_handle<System::Time>(m_content));

  file_io->read_config();

  m_systems[1]->init(); // init graphics

  file_io->read_init (folder_name);

  m_systems[4]->init(); // init interface

  m_content.set<Component::Event>("music:start");
  m_content.set<Component::Event>("window:rescaled");

  while (!m_content.request<Component::Event>("game:exit"))
    for (System::Handle system : m_systems)
      system->run();

  file_io->write_config();

  return EXIT_SUCCESS;
}



} // namespace Sosage
