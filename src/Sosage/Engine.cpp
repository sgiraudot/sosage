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
#include <Sosage/Component/Console.h>
#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Config/platform.h>
#include <Sosage/Engine.h>
#include <Sosage/Utils/profiling.h>
#include <Sosage/Utils/file.h>

namespace Sosage
{

Engine::Engine (const std::string& game_name)
  : m_animation (m_content)
#ifdef SOSAGE_ANDROID
  , m_graphic (m_content, game_name, 1600, 1600 * 10 / 16, true)
#else
  , m_graphic (m_content, game_name, 1600, 1600 * 10 / 16, false)
#endif
  , m_sound (m_content)
  , m_input (m_content)
  , m_interface (m_content)
  , m_file_io (m_content)
  , m_logic (m_content)
{
  srand(time(nullptr));
}

Engine::~Engine()
{
  // Clear content before shutting down systems
  m_content.clear();
}

void Engine::run()
{
  std::size_t frame_id = m_clock.frame_id();

  m_content.set<Component::Event>("music:start");
  m_content.set<Component::Event>("window:rescaled");

  while (!m_logic.exit())
  {
    m_input.run();
    std::size_t new_frame_id = m_clock.frame_id();
    if (!m_logic.paused())
    {
      m_logic.run(m_clock.frame_time());
      m_interface.run();
      if (new_frame_id != frame_id)
        for (std::size_t i = frame_id; i < new_frame_id; ++ i)
          m_animation.run();
    }
    m_sound.run();
    frame_id = new_frame_id;
    m_graphic.run();
    m_clock.wait(true);
  }
}

int Engine::run (const std::string& folder_name)
{
  m_content.set<Component::Console>("game:console");
  m_content.set<Component::Int>("interface:width", 0);
  m_content.set<Component::Int>("interface:height", 200);
  m_content.set<Component::Int>("window:width", 1600);
  m_content.set<Component::Int>("window:height", 1600 * 10 / 16);
  m_content.set<Component::Int>("text:char_per_second", 12);

  std::string room_name = m_file_io.read_init (folder_name);

  // Create debug info
  auto debug_info = m_content.set<Component::Debug>("game:debug", m_content, m_clock);

  // Create inventory
  auto inventory = m_content.set<Component::Inventory>("game:inventory");

  m_interface.init();

  debug("3");
  while (room_name != std::string())
  {
    room_name = m_file_io.read_room (room_name);
    m_animation.generate_random_idle_animation
      (m_content.get<Component::Animation>("character_body:image"),
       m_content.get<Component::Animation>("character_head:image"),
       m_content.get<Component::Animation>("character_mouth:image"),
       Vector(1,0));
    
    run();
  }
  
  return EXIT_SUCCESS;
}



} // namespace Sosage
