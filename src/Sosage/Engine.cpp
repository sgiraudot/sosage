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
#include <Sosage/System/Control.h>
#include <Sosage/System/File_IO.h>
#include <Sosage/System/Graphic.h>
#ifdef SOSAGE_DEV
#include <Sosage/System/Test_input.h>
#endif
#include <Sosage/System/Input.h>
#include <Sosage/System/Interface.h>
#include <Sosage/System/Logic.h>
#include <Sosage/System/Menu.h>
#include <Sosage/System/Sound.h>
#include <Sosage/System/Time.h>
#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/error.h>
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
#ifdef SOSAGE_DEBUG_BUFFER
Debug_buffer debug_buffer;
std::ostream debug(&debug_buffer);
#endif

#ifdef SOSAGE_EMSCRIPTEN
Engine* emscripten_global_engine_ptr;
void emscripten_main_loop()
{
  emscripten_global_engine_ptr->run();
}
#endif

Engine::Engine (int argc, char** argv)
{
  SOSAGE_UPDATE_DBG_LOCATION("Engine::Engine()");
  debug << "Running Sosage " << Sosage::Version::str() << std::endl;
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

bool Engine::run (const std::string& folder_name)
{
  if (!Asset_manager::init(folder_name))
  {
    debug << "Asset manager could not use " << folder_name << std::endl;
    return false;
  }

  // Init main variables
  auto status = m_content.set_fac<Component::Status>(GAME__STATUS, "Game", "status");
  m_content.set_fac<Component::Absolute_position>(CAMERA__POSITION, "Camera", "position", Point(0,0));
  m_content.set_fac<Component::Double>(CAMERA__ZOOM, "Camera", "zoom", 1.);
  m_content.set<Component::Inventory>("Game", "inventory");
  m_content.set<Component::Set<std::string> > ("Game", "visited_rooms");
  m_content.emit("Game", "just_launched");

  m_content.set<Component::And>
      ("Unlocked", "condition",
       Component::make_not
        (Component::make_value_condition<Sosage::Status> (status, PAUSED)),
        Component::make_not
        (Component::make_value_condition<Sosage::Status> (status, CUTSCENE)));

  auto file_io = System::make_handle<System::File_IO>(m_content);
  auto graphic = System::make_handle<System::Graphic>(m_content);
  auto control = System::make_handle<System::Control>(m_content);
  auto interface = System::make_handle<System::Interface>(m_content);
  auto menu = System::make_handle<System::Menu>(m_content);
  auto time = System::make_handle<System::Time>(m_content);
  auto animation = System::make_handle<System::Animation>(m_content);

  // Create all systems
  m_systems.push_back (file_io);
#ifdef SOSAGE_DEV
  if (m_input_mode == NORMAL)
#endif
    m_systems.push_back (System::make_handle<System::Input>(m_content));
#ifdef SOSAGE_DEV
  else
  {
    auto input = System::make_handle<System::Test_input>(m_content);
    if (m_input_mode == TEST_RANDOM)
      input->set_random_mode();
    m_systems.push_back (input);
  }
#endif

  m_systems.push_back (control);
  m_systems.push_back (interface);
  m_systems.push_back (menu);
  m_systems.push_back (System::make_handle<System::Logic>(m_content));
  m_systems.push_back (animation);
  m_systems.push_back (System::make_handle<System::Sound>(m_content));
  m_systems.push_back (graphic);
  m_systems.push_back (time);

  file_io->read_config();

  graphic->init(); // init graphics
  m_content.emit ("Window", "rescaled");
  graphic->run(); // Run graphic once to update view

  m_content.set<Component::Simple<std::function<void()> > >
      ("Game", "loading_callback",
       [&]()
       {
#ifndef SOSAGE_EMSCRIPTEN
          time->run();
          if (animation->run_loading())
            graphic->run_loading();
#endif
       });


  file_io->read_init ();
  control->init();
  interface->init();
  menu->init();

  debug << "Init done, entering main loop" << std::endl;

#ifdef SOSAGE_EMSCRIPTEN
  emscripten_set_main_loop (emscripten_main_loop, 0, 0);
  emscripten_exit_with_live_runtime();
#else
  while (run()) { }
#endif

  file_io->write_config();
  if (m_content.receive("Game", "save"))
    file_io->write_savefile();

  m_systems.clear();
  interface.reset(); // Clear interface before SDL is exited
  m_content.clear();

  return true;
}

bool Engine::run()
{
  for (System::Handle system : m_systems)
    system->run();
  return !m_content.receive("Game", "exit");
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
      m_content.set<Component::String>("Cmdline", "locale", argv[i]);
    }
#ifdef SOSAGE_DEV
    else if (arg == "--no-exit" || arg == "-e")
      m_content.emit("Game", "prevent_exit");
    else if (arg == "--no-restart" || arg == "-n")
      m_content.emit("Game", "prevent_restart");
    else if (arg == "--mouse" || arg == "-m")
       m_input_mode = TEST_MOUSE;
    else if (arg == "--test" || arg == "-t")
       m_input_mode = TEST_RANDOM;
    else if (arg == "--save" || arg == "-s")
    {
      ++ i;
      if (i == argc)
        break;
      m_content.set<Component::String>("Save", "suffix", argv[i]);
    }
    else if (arg == "--room" || arg == "-r")
    {
      ++ i;
      if (i == argc)
        break;
      m_content.set<Component::String>("Force_load", "room", argv[i]);
      ++ i;
      if (i == argc)
        break;
      if (argv[i][0] == '-')
        -- i;
      else
        m_content.set<Component::String>("Force_load", "origin", argv[i]);
    }
#endif
  }
}

} // namespace Sosage
