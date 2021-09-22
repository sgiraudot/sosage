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
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Config/options.h>
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
#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/profiling.h>

#include <ctime>

#ifdef SOSAGE_EMSCRIPTEN
#include <emscripten.h>
#endif

namespace Sosage
{

// Global variables
std::string Asset_manager::folder_name = "";
std::vector<Buffer> Asset_manager::buffers;
Package_asset_map Asset_manager::package_asset_map;

#ifdef SOSAGE_EMSCRIPTEN
Engine* emscripten_global_engine_ptr;
void emscripten_main_loop()
{
  emscripten_global_engine_ptr->run();
}
#endif

Engine::Engine (int argc, char** argv)
{
  debug ("Running Sosage ", Sosage::Version::str());
  srand(static_cast<unsigned int>(time(nullptr)));

  handle_cmdline_args(argc, argv);

#ifdef SOSAGE_EMSCRIPTEN
  emscripten_global_engine_ptr = this;
#endif
}

Engine::~Engine()
{
  // Clear content before shutting down systems
  m_content.clear();
}

int Engine::run (const std::string& folder_name)
{
  Asset_manager::init(folder_name);

  // Init main variables
  auto status = m_content.set_fac<Component::Status>(GAME__STATUS, "Game:status");
  m_content.set_fac<Component::Absolute_position>(CAMERA__POSITION, "Camera:position", Point(0,0));
  m_content.set<Component::Double>("Camera:target", 0.0);
  m_content.set<Component::Inventory>("Game:inventory");
  m_content.set<Component::Set<std::string> > ("Game:visited_rooms");

  m_content.set<Component::And>
      ("Unlocked:condition",
       Component::make_not
        (Component::make_value_condition<Sosage::Status> (status, PAUSED)),
        Component::make_not
        (Component::make_value_condition<Sosage::Status> (status, CUTSCENE)));

  auto file_io = System::make_handle<System::File_IO>(m_content);
  auto graphic = System::make_handle<System::Graphic>(m_content);
  auto interface = System::make_handle<System::Interface>(m_content);
  auto time = System::make_handle<System::Time>(m_content);
  auto animation = System::make_handle<System::Animation>(m_content);

  // Create all systems
  m_systems.push_back (file_io);
  m_systems.push_back (System::make_handle<System::Input>(m_content));
  m_systems.push_back (interface);
  m_systems.push_back (System::make_handle<System::Logic>(m_content));
  m_systems.push_back (animation);
  m_systems.push_back (System::make_handle<System::Sound>(m_content));
  m_systems.push_back (graphic);
  m_systems.push_back (time);

  file_io->read_config();

  graphic->init(); // init graphics
  m_content.emit ("Window:rescaled");
  graphic->run(); // Run graphic once to update view

  m_content.set<Component::Simple<std::function<void()> > >
      ("Game:loading_callback",
       [&]()
       {
#ifndef SOSAGE_EMSCRIPTEN
          time->run_loading();
          if (animation->run_loading())
            graphic->run_loading();
#endif
       });


  file_io->read_init ();
  interface->init(); // init interface

  debug("Init done, entering main loop");

#ifdef SOSAGE_EMSCRIPTEN
  emscripten_set_main_loop (emscripten_main_loop, 0, 0);
  emscripten_exit_with_live_runtime();
#else
  while (run()) { }
#endif

  file_io->write_config();
  if (m_content.receive("Game:save"))
    file_io->write_savefile();

  m_systems.clear();
  interface.reset(); // Clear interface before SDL is exited
  m_content.clear();

  return EXIT_SUCCESS;
}

bool Engine::run()
{
#ifdef SOSAGE_ASSERTIONS_IN_DIALOG
  try
  {
#endif
    for (System::Handle system : m_systems)
      system->run();
#ifdef SOSAGE_ASSERTIONS_IN_DIALOG
  }
  catch (std::runtime_error& error)
  {
    std::dynamic_pointer_cast<System::Graphic>(m_systems[6])->display_error (error.what());
    throw error;
  }
#endif

  return !m_content.receive("Game:exit");
}

void Engine::handle_cmdline_args (int argc, char** argv)
{
  for (int i = 1; i < argc; ++ i)
  {
    std::string arg (argv[i]);
    if (arg == "--locale" || arg == "-l")
    {
      ++ i;
      if (i == argc)
        break;
      m_content.set<Component::String>("Cmdline:locale", argv[i]);
    }
  }
}

} // namespace Sosage
