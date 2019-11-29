#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/Engine.h>
#include <Sosage/platform.h>
#include <Sosage/Utils/profiling.h>

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

void Engine::main()
{
  std::size_t frame_id = m_clock.frame_id();
  
  while (!m_logic.exit())
  {
    m_input.main();
    std::size_t new_frame_id = m_clock.frame_id();
    if (!m_logic.paused())
    {
      m_logic.main();
      m_interface.main();
      if (new_frame_id != frame_id)
        for (std::size_t i = frame_id; i < new_frame_id; ++ i)
          m_animation.main();
      m_sound.main();
    }
    frame_id = new_frame_id;
    m_graphic.main();
    m_clock.wait(true);
  }
}

int Engine::run (const std::string& folder_name)
{
  std::string room_name = m_io.read_init (folder_name);

  // Create debug info
  Component::Debug_handle debug_info
    = m_content.set<Component::Debug>("game:debug", m_content, m_clock);

  m_interface.init();

#ifdef SOSAGE_ANDROID
  m_graphic.set_cursor (m_content.get<Component::Text> ("cursor:path")->value());
#endif

  while (room_name != std::string())
  {
    room_name = m_io.read_room (room_name);
    m_animation.generate_random_idle_animation
      (m_content.get<Component::Animation>("character_body:image"),
       m_content.get<Component::Animation>("character_head:image"), Vector(1,0));
    
    main();
  }
  
  return EXIT_SUCCESS;
}



} // namespace Sosage
