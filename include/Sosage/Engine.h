#ifndef SOSAGE_ENGINE_H
#define SOSAGE_ENGINE_H

#include <Sosage/Content.h>
#include <Sosage/System/Graphic.h>
#include <Sosage/System/Sound.h>
#include <Sosage/System/Input.h>
#include <Sosage/System/Logic.h>
#include <Sosage/Third_party/Lua.h>

namespace Sosage
{

class Engine
{
  typedef Third_party::Lua Core;
  
  Content m_content;
  System::Graphic m_graphic;
  System::Sound m_sound;
  System::Input m_input;
  System::Logic m_logic;

  std::string m_file_name;
  Core m_core;

public:

  Engine (const std::string& game_name);

  void main ();

  int run_file (const std::string& file_name);
  
  int run_directory (const std::string& directory_name);

  void set_image (const std::string& key, const std::string& file_name, int x, int y, int z);
  void set_ground_map (const std::string& key, const std::string& file_name);

private:
  
  inline std::string local_file_name (const std::string& file_name)
  {
    std::size_t last = m_file_name.find_last_of('/');
    if (last == std::string::npos)
      return file_name;

    return std::string(m_file_name.begin(), m_file_name.begin() + last + 1) + file_name;
  }

};

inline Engine& engine (const std::string& game_name = std::string())
{
  static std::unique_ptr<Engine> e;
  if (game_name != std::string())
    e = std::unique_ptr<Engine>(new Engine(game_name));
  return *e;
}

} // namespace Sosage

#endif // SOSAGE_ENGINE_H
