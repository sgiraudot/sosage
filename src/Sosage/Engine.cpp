#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/Engine.h>
#include <Sosage/Utils/profiling.h>

namespace Sosage
{

Engine::Engine (const std::string& game_name)
  : m_animation (m_content)
  , m_graphic (m_content, game_name)
  , m_sound (m_content)
  , m_input (m_content)
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

  init_interface();

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

void Engine::init_interface ()
{
  m_content.set<Component::Image> ("interface:image", 1920, 200);
  m_content.set<Component::Position> ("interface:position", Point(0,880));
  
  Component::Font_handle interface_font = m_content.get<Component::Font> ("interface:font");
  std::string color_str = m_content.get<Component::Text> ("interface:color")->value();

  for (const auto& verb : { std::make_tuple (std::string("open"), std::string("Ouvrir"), 145, 960),
        std::make_tuple (std::string("close"), std::string("Fermer"), 145, 1030),
        std::make_tuple (std::string("give"), std::string("Donner"), 398, 960),
        std::make_tuple (std::string("take"), std::string("Prendre"), 398, 1030),
        std::make_tuple (std::string("look"), std::string("Regarder"), 674, 960),
        std::make_tuple (std::string("talk"), std::string("Parler"), 674, 1030),
        std::make_tuple (std::string("use"), std::string("Utiliser"), 960, 960),
        std::make_tuple (std::string("move"), std::string("Déplacer"), 960, 1030) })
  {
    m_content.set<Component::Text> ("verb_" + std::get<0>(verb) + ":text", std::get<1>(verb));
    Component::Image_handle verb_img
      = m_content.set<Component::Image> ("verb_" + std::get<0>(verb) + ":image",
                                         interface_font, color_str, std::get<1>(verb));
    verb_img->set_relative_origin(0.5, 0.5);
    verb_img->set_scale(0.75);
    Component::Position_handle verb_pos
      = m_content.set<Component::Position>("verb_" + std::get<0>(verb) + ":position",
                                           Point(std::get<2>(verb), std::get<3>(verb)));
  }

  m_content.set<Component::Text> ("verb_goto:text", "Aller vers");
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
  pause_text_img->set_relative_origin(0.5, 0.5);

  Component::Conditional_handle pause_text
    = m_content.set<Component::Conditional>("pause_text:conditional", paused,
                                            pause_text_img, Component::Handle());
    
  m_content.set<Component::Position>("pause_text:position", Point(config().world_width / 2,
                                                                  config().world_height / 2));

}



} // namespace Sosage
