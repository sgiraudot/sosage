#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Third_party/XML.h>
#include <Sosage/Engine.h>

namespace Sosage
{

Engine::Engine (const std::string& game_name)
  : m_animation (m_content)
  , m_graphic (m_content, game_name)
  , m_sound (m_content)
  , m_input (m_content)
  , m_logic (m_content)
{
  srand(time(nullptr));
}

void Engine::main()
{
  while (!m_logic.exit())
  {
    m_input.main();
    m_logic.main();
    m_animation.main();
    m_graphic.main();
    m_sound.main();
    m_clock.wait(true);
  }
}

int Engine::read_file (const std::string& file_name)
{
  m_file_name = file_name;
  Third_party::XML().read (file_name, *this);
  main();
  return EXIT_SUCCESS;
}

void Engine::set_background (const std::string& image, const std::string& ground_map)
{
  std::cerr << "Setting background " << image << " with ground_map " << ground_map << std::endl;

  m_content.set<Component::Image>("background:image", local_file_name(image), 0);
  m_content.set<Component::Path>("background:position", Point(0, 0, WORLD));
  m_content.set<Component::Ground_map>("background:ground_map", local_file_name(ground_map));
}

void Engine::set_character (const std::string& body, const std::string& head, int x, int y)
{
  std::cerr << "Setting character " << body << "/" << head << " at " << x << "*" << y << std::endl;

  Component::Animation_handle abody
     = m_content.set<Component::Animation>("character:image", local_file_name(body),
                                           0, 9, 5);
  Component::Animation_handle ahead
     = m_content.set<Component::Animation>("character:head", local_file_name(head),
                                           0, 6, 2);
  Component::Path_handle position
    = m_content.set<Component::Path>("character:position", Point(x, y, WORLD));
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");
  
  Vector translation (abody->width() / 2,
                      abody->height(), CAMERA);
  Point pos = (*position)[0] + translation;
  abody->rescale (ground_map->z_at_point (pos));
  ahead->rescale (ground_map->z_at_point (pos));
  ahead->z() += 1;
  Vector back_translation (abody->width() / 2,
                           abody->height(), CAMERA);
  (*position)[0] = pos - back_translation;

  m_logic.generate_random_idle_animation(abody, ahead, Vector(1,0,WORLD));
}

} // namespace Sosage
