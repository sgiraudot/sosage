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

  std::string m_file_name;

public:

  Engine (const std::string& game_name);

  void main ();

  int read_file (const std::string& file_name);

  void set_background (const std::string& image, const std::string& ground_map);
  void set_character (const std::string& body, const std::string& head, int x, int y);

private:
  
  inline std::string local_file_name (const std::string& file_name)
  {
    std::size_t last = m_file_name.find_last_of('/');
    if (last == std::string::npos)
      return file_name;

    return std::string(m_file_name.begin(), m_file_name.begin() + last + 1) + file_name;
  }

};

} // namespace Sosage

#endif // SOSAGE_ENGINE_H
