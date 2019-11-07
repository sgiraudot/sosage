#ifndef SOSAGE_THIRD_PARTY_SDL_H
#define SOSAGE_THIRD_PARTY_SDL_H

#include <Sosage/Utils/error.h>
#include <Sosage/Config.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

namespace Sosage::Third_party
{

class SDL
{
public:

  typedef std::pair<SDL_Texture*, double> Image;
  typedef SDL_Surface* Surface;
  
  static SDL_Window* m_window;
  static SDL_Renderer* m_renderer;

  
public:

  SDL (const std::string& game_name, int width, int height, bool fullscreen);
  ~SDL ();

  static Surface load_surface (const std::string& file_name)
  {
    SDL_Surface* surf = IMG_Load (file_name.c_str());
    check (surf != nullptr, "Cannot load image " + file_name);
    return surf;
  }

  static Image load_image (const std::string& file_name)
  {
    SDL_Surface* surf = load_surface (file_name);
    SDL_Texture* out = SDL_CreateTextureFromSurface (m_renderer, surf);
    check (out != nullptr, "Cannot create texture from " + file_name);
    SDL_FreeSurface (surf);
    return std::make_pair (out, 1);
  }

  static void rescale (Image& source, double scaling)
  {
    source.second = scaling;
  }

  static void delete_image (const Image& source)
  {
    SDL_DestroyTexture (source.first);
  }

  static std::array<unsigned char, 3> get_color (Surface image, int x, int y)
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
  static int width (Surface image) { return image->w; }
  static int height (Surface image) { return image->h; }
  static int width (Image image)
  {
    int out;
    SDL_QueryTexture (image.first, NULL, NULL, &out, NULL);
    return out;
  }
  static int height (Image image)
  {
    int out;
    SDL_QueryTexture (image.first, NULL, NULL, NULL, &out);
    return out;
  }

  void begin();
  void draw (const Image& image,
             const int xsource, const int ysource,
             const int wsource, const int hsource,
             const int xtarget, const int ytarget,
             const int wtarget, const int htarget);

  void draw_line (const int xa, const int ya, const int xb, const int yb);
  void end();

};

} // namespace Sosage::Third_party

#endif // SOSAGE_THIRD_PARTY_SDL_H
