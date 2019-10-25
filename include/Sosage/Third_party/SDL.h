#ifndef SOSAGE_THIRD_PARTY_SDL_H
#define SOSAGE_THIRD_PARTY_SDL_H

#ifdef SOSAGE_LINKED_WITH_SDL

#include <Sosage/Utils/error.h>
#include <Sosage/Config.h>

#include <SDL/SDL.h>
#include <SDL/SDL_image.h>
#include <SDL/SDL_gfxPrimitives.h>
#include <SDL/SDL_rotozoom.h>
#include <SDL/SDL_ttf.h>

namespace Sosage::Third_party
{

class SDL
{
public:

  typedef SDL_Surface* Image;
  
private:

  SDL_Surface* m_window;
  
public:

  SDL (const std::string& game_name, int width, int height, bool fullscreen);
  ~SDL ();

  static Image load_image (const std::string& file_name)
  {
    SDL_Surface* out = IMG_Load (file_name.c_str());
    
    check (out != nullptr, "Cannot load image " + file_name);

    if (config().world_height != config().camera_height)
    {
      SDL_Surface* scaled = zoomSurface (out, 1. / config().camera_scaling,
                                         1. / config().camera_scaling, 1);
      std::swap (out, scaled);
      SDL_FreeSurface (scaled);
    }
    
    return out;
  }

  static Image copy_image (const Image& source)
  {
    return zoomSurface (source, 1, 1, 1);
  }

  static Image rescale (const Image& source, double scaling)
  {
    return zoomSurface (source, scaling, scaling, 1);
  }

  static void delete_image (const Image& source)
  {
    SDL_FreeSurface (source);
  }

  static std::array<unsigned char, 3> get_color (Image image, int x, int y)
  {
    SDL_LockSurface (image);

    int bpp = image->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)image->pixels + y * image->pitch + x * bpp;
    Uint32 data;
    switch (bpp)
    {
      case 1:
        data = *p;
        break;

      case 2:
        data = *(Uint16 *)p;
        break;

      case 3:
        if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
          data = p[0] << 16 | p[1] << 8 | p[2];
        else
          data = p[0] | p[1] << 8 | p[2] << 16;
        break;

      case 4:
        data = *(Uint32 *)p;
        break;

      default:
        exit(0);
    }
    std::array<unsigned char, 3> out;
    SDL_GetRGB(data, image->format, &out[0], &out[1], &out[2]);
    SDL_UnlockSurface (image);
    return out;
  }
  static int width (Image image) { return image->w; }
  static int height (Image image) { return image->h; }

  void begin();
  void draw (const Image& image, const int x, const int y,
             const int xmin, const int xmax,
             const int ymin, const int ymax);
  void draw_line (const int xa, const int ya, const int xb, const int yb);
  void end();

};

} // namespace Sosage::Third_party

#endif

#endif // SOSAGE_THIRD_PARTY_SDL_H
