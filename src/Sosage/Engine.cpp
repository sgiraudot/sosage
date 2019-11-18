#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Third_party/XML.h>
#include <Sosage/Engine.h>
#include <Sosage/Utils/profiling.h>

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

Engine::~Engine()
{
  // Clear content before shutting down systems
  m_content.clear();
}

void Engine::main()
{
  while (!m_logic.exit())
  {
    m_input.main();
    if (!m_logic.paused())
    {
      m_logic.main();
      m_animation.main();
      m_sound.main();
    }
    m_graphic.main();
    m_clock.wait(true);
  }
}

int Engine::run (const std::string& folder_name)
{
  m_folder_name = folder_name;
  std::string file_name = folder_name + "init.xml";
  std::string room_name = Third_party::XML().read (file_name, *this);

  while (room_name != std::string())
  {
    room_name = Third_party::XML().read (local_file_name(room_name), *this);
    main();
  }
  
  return EXIT_SUCCESS;
}

void Engine::set_background (const std::string& image, const std::string& ground_map)
{
  debug("Setting background " + image + " with ground_map " + ground_map);

  m_content.set<Component::Image>("background:image", local_file_name(image), 0);
  m_content.set<Component::Position>("background:position", Point(0, 0, WORLD));
  m_content.set<Component::Ground_map>("background:ground_map", local_file_name(ground_map));
}

void Engine::set_interface (const std::string& cursor,
                            const std::string& dbg_font,
                            const std::string& font,
                            const std::string& color_str)
{
  m_graphic.set_cursor (local_file_name(cursor));
  
  m_content.set<Component::Image> ("interface:image", 1080, 200);
  m_content.set<Component::Position> ("interface:position", Point(0,880));
  
  Component::Font_handle interface_font
    = m_content.set<Component::Font> ("interface:font", local_file_name(font), 80);
  m_content.set<Component::Font> ("debug:font", local_file_name(dbg_font), 15);

  for (const auto& verb : { std::make_tuple (std::string("open"), std::string("Ouvrir"), 145, 960),
        std::make_tuple (std::string("close"), std::string("Fermer"), 145, 1030),
        std::make_tuple (std::string("give"), std::string("Donner"), 398, 960),
        std::make_tuple (std::string("take"), std::string("Prendre"), 398, 1030),
        std::make_tuple (std::string("look"), std::string("Regarder"), 674, 960),
        std::make_tuple (std::string("talk"), std::string("Parler"), 674, 1030),
        std::make_tuple (std::string("use"), std::string("Utiliser"), 960, 960),
        std::make_tuple (std::string("move"), std::string("Déplacer"), 960, 1030) })
  {
    Component::Image_handle verb_img
      = m_content.set<Component::Image> ("verb_" + std::get<0>(verb) + ":image",
                                         interface_font, color_str, std::get<1>(verb));
    verb_img->origin() = Point (verb_img->width() / 2, verb_img->height() / 2);
    verb_img->set_scale(0.75);
    Component::Position_handle verb_pos
      = m_content.set<Component::Position>("verb_" + std::get<0>(verb) + ":position",
                                           Point(std::get<2>(verb), std::get<3>(verb)));
  }

  Component::Image_handle text_img
    = m_content.set<Component::Image>("chosen_verb:image", interface_font, "FFFFFF", "Aller vers");
  text_img->origin() = Point (text_img->width() / 2, text_img->height() / 2);
  text_img->set_scale(0.5);
  m_content.set<Component::Position>("chosen_verb:position", Point(960, 905));

  // Create debug info
  Component::Debug_handle debug_info
    = m_content.set<Component::Debug>("game:debug", m_content, m_clock);

  // Create pause screen
  Component::Boolean_handle paused
    = m_content.set<Component::Boolean>("game:paused", false);

  Component::Image_handle pause_screen_img
    = Component::make_handle<Component::Image>
    ("pause_screen:image", config().world_width, config().world_height,
     0, 0, 0, 192);
  pause_screen_img->z() += 10;
      
  Component::Conditional_handle pause_screen
    = m_content.set<Component::Conditional>("pause_screen:conditional", paused,
                                            pause_screen_img, Component::Handle());
  
  m_content.set<Component::Position>("pause_screen:position", Point(0, 0));

  Component::Image_handle pause_text_img
    = Component::make_handle<Component::Image>("pause_text:image", interface_font, "FFFFFF", "PAUSE");
  pause_text_img->z() += 10;
  pause_text_img->origin() = Point (pause_text_img->width() / 2, pause_text_img->height() / 2);

  Component::Conditional_handle pause_text
    = m_content.set<Component::Conditional>("pause_text:conditional", paused,
                                            pause_text_img, Component::Handle());
    
  m_content.set<Component::Position>("pause_text:position", Point(config().world_width / 2,
                                                                  config().world_height / 2));

}

void Engine::set_character (const std::string& body, const std::string& head, int x, int y)
{
  debug ("Setting character " + body + "/" + head + " at " + std::to_string(x)
         + "*" + std::to_string(y));

  Component::Animation_handle abody
     = m_content.set<Component::Animation>("character_body:image", local_file_name(body),
                                           0, 9, 5);
  abody->origin() = Point (abody->width() / 2, 0.9 * abody->height());
  
  Component::Animation_handle ahead
     = m_content.set<Component::Animation>("character_head:image", local_file_name(head),
                                           0, 7, 2);
  ahead->origin() = Point (ahead->width() / 2, ahead->height());
  
  Component::Position_handle pbody
    = m_content.set<Component::Position>("character_body:position", Point(x, y));

  
  Component::Position_handle phead
    = m_content.set<Component::Position>("character_head:position", Point(x, y - 290));
  
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");
  
  Point pos_body = pbody->value();

  // abody->z() = 1;
  // ahead->z() = 2;
  double z_at_point = ground_map->z_at_point (pos_body);
  abody->rescale (z_at_point);
  ahead->rescale (z_at_point);
  ahead->z() += 1;
  phead->set (pbody->value() - abody->core().second * Vector(0, 290));

  m_logic.generate_random_idle_animation(abody, ahead, Vector(1,0));
}

void Engine::add_object (const std::string& id, const std::string& image, int x, int y)
{
  Component::Image_handle img
    = m_content.set<Component::Image>(id + ":image", local_file_name(image), 0);
  img->origin() = Point (img->width() / 2, img->height());

  Component::Position_handle pos
    = m_content.set<Component::Position>(id + ":position", Point(x + img->width() / 2, y + img->height()));
  
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");
  img->z() = ground_map->z_at_point (pos->value()) - 1;
  debug("Object " + id + " at position " + std::to_string(img->z()));
}


} // namespace Sosage
