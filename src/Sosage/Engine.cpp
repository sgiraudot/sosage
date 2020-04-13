#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Console.h>
#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/Engine.h>
#include <Sosage/platform.h>
#include <Sosage/Utils/profiling.h>
#include <Sosage/Utils/file.h>

namespace Sosage
{

Engine::Engine (const std::string& game_name)
  : m_animation (m_content)
  , m_graphic (m_content, game_name)
  , m_sound (m_content)
  , m_input (m_content)
  , m_interface (m_content)
  , m_io (m_content)
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

  std::string room_name = m_io.read_init (folder_name);

  // Create debug info
  auto debug_info = m_content.set<Component::Debug>("game:debug", m_content, m_clock);

  // Create inventory
  auto inventory = m_content.set<Component::Inventory>("game:inventory");

  m_interface.init();

  debug("3");
  while (room_name != std::string())
  {
    room_name = m_io.read_room (room_name);
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
