#ifndef SOSAGE_MODELS_GRAPHIC_CORE_H
#define SOSAGE_MODELS_GRAPHIC_CORE_H

#include <Sosage/Utils/error.h>

namespace Sosage::Models
{

class Graphic_core
{
public:

  typedef int Image;
  
public:

  Graphic_core (const std::string& game_name, int width, int height, bool full_screen)
  {
    debug ("Graphic_core (" + game_name
                          + ", " + std::to_string(width)
                          + ", " + std::to_string(height) + ", "
                          + std::to_string(full_screen) + ");");
  }

  ~Graphic_core ()
  {
    debug ("~Graphic_core ();");
  }

  static Image load_image (const std::string& file_name)
  {
    debug ("Graphic_core::load_image (" + file_name + ");");
    return 0;
  }

  static std::array<unsigned char,3> get_color (Image, int x, int y)
  {
    debug ("Graphic_core::get_color (" + std::to_string(x) + ", " + std::to_string(y) + ");");
    return std::array<unsigned char, 3>{0,0,0};
  }
  static int width (Image) { return 0; }
  static int height (Image) { return 0; }

  void begin()
  {
    debug ("Graphic_core::begin ();");
  }

  void draw (const Image& image, const int x, const int y)
  {
    debug ("Graphic_core::draw ();");
  }

  void draw_line (const int xa, const int ya, const int xb, const int yb)
  {
    debug ("Graphic_core::draw_line ();");
  }

  void end()
  {
    debug ("Graphic_core::end ();");
  }
   
};

} // namespace Sosage::Models

#endif // SOSAGE_MODELS_GRAPHIC_CORE_H
