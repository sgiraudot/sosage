#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Engine.h>

namespace Sosage
{

Engine::Engine (const std::string& game_name)
  : m_graphic (m_content, game_name)
  , m_sound (m_content)
  , m_input (m_content)
  , m_logic (m_content)
{

}

void Engine::main()
{
  std::thread graphic_thread ([&]() { m_graphic.main(); });
  std::thread sound_thread ([&]() { m_sound.main(); });
  std::thread input_thread ([&]() { m_input.main(); });
  std::thread logic_thread ([&]() { m_logic.main(); });
  
  input_thread.join();

  // Once input thread has stopped, stop the others
  m_graphic.stop();
  m_logic.stop();
  m_sound.stop();
  
  graphic_thread.join();
  sound_thread.join();
  logic_thread.join();
}

int Engine::run_file (const std::string& file_name)
{
  m_file_name = file_name;
  m_core.read (file_name);
  main();
  return EXIT_SUCCESS;
}

int Engine::run_directory (const std::string& directory_name)
{

  return EXIT_SUCCESS;
}


void Engine::set_image (const std::string& key,
                                const std::string& file_name,
                                int x, int y, int z)
{
  std::cerr << "Setting image " << key << " to " << file_name << std::endl;

  m_content.set<Component::Image>(key, "image", local_file_name(file_name), z);
  m_content.set<Component::Path>(key, "position", Point(x, y, WORLD));
}

void Engine::set_ground_map (const std::string& key,
                                     const std::string& file_name)
{
  std::cerr << "Setting ground map " << key << " to " << file_name << std::endl;

  m_content.set<Component::Ground_map>(key, "ground_map", local_file_name(file_name));
}

} // namespace Sosage
