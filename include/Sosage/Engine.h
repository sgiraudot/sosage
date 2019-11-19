#ifndef SOSAGE_ENGINE_H
#define SOSAGE_ENGINE_H

#include <Sosage/Content.h>
#include <Sosage/System/Animation.h>
#include <Sosage/System/Graphic.h>
#include <Sosage/System/Sound.h>
#include <Sosage/System/Input.h>
#include <Sosage/System/IO.h>
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
  System::IO m_io;
  System::Logic m_logic;
  Clock m_clock;

public:

  Engine (const std::string& game_name);
  ~Engine();

  void main ();

  int run (const std::string& folder_name);
  
  void init_interface ();
};

} // namespace Sosage

#endif // SOSAGE_ENGINE_H
