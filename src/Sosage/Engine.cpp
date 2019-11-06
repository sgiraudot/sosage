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
     = m_content.set<Component::Animation>("character_body:image", local_file_name(body),
                                           0, 9, 5);
  Component::Animation_handle ahead
     = m_content.set<Component::Animation>("character_head:image", local_file_name(head),
                                           0, 7, 2);
  Component::Path_handle pbody
    = m_content.set<Component::Path>("character_body:position", Point(x, y, WORLD));

  abody->z() = 1;
  
  Component::Path_handle phead
    = m_content.set<Component::Path>("character_head:position", Point(x + 66, y, WORLD));
//    = m_content.set<Component::Path>("character_head:position", Point(x, y, WORLD));
  
  ahead->z() = 2;
  
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");
  
  Vector tbody (abody->width() / 2,
                abody->height(), CAMERA);
  
  Point pos_body = (*pbody)[0] + tbody;

  double z_at_point = ground_map->z_at_point (pos_body);
  abody->rescale (z_at_point);
  ahead->rescale (z_at_point);
  ahead->z() += 1;
    
  Vector btbody (abody->width() / 2,
                 abody->height(), CAMERA);
  
  (*pbody)[0] = pos_body - btbody;
  (*phead)[0] = (*pbody)[0] + (1. / (z_at_point / double(config().world_depth))) * Vector (66, 0, WORLD);

  m_logic.generate_random_idle_animation(abody, ahead, Vector(1,0,WORLD));
}

} // namespace Sosage
