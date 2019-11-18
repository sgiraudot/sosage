#ifndef SOSAGE_ENGINE_H
#define SOSAGE_ENGINE_H

#include <Sosage/Content.h>
#include <Sosage/System/Animation.h>
#include <Sosage/System/Graphic.h>
#include <Sosage/System/Sound.h>
#include <Sosage/System/Input.h>
#include <Sosage/System/Logic.h>
#include <Sosage/Utils/time.h>

namespace Sosage
{

class Engine
{
  Content m_content;
  System::Animation m_animation;
  System::Graphic m_graphic;
  System::Sound m_sound;
  System::Input m_input;
  System::Logic m_logic;
  Clock m_clock;

  std::string m_folder_name;

public:

  Engine (const std::string& game_name);
  ~Engine();

  void main ();

  int run (const std::string& folder_name);
  
  void set_interface (const std::string& cursor, const std::string& dbg_font,
                      const std::string& font, const std::string& color_str);
  void set_background (const std::string& image, const std::string& ground_map);
  void set_character (const std::string& body, const std::string& head, int x, int y);

  void add_object (const std::string& id, const std::string& image, int x, int y);

private:
  
  inline std::string local_file_name (const std::string& file_name)
  {
    return m_folder_name + file_name;
  }

};

} // namespace Sosage

#endif // SOSAGE_ENGINE_H
