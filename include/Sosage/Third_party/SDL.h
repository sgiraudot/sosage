#ifndef SOSAGE_THIRD_PARTY_SDL_H
#define SOSAGE_THIRD_PARTY_SDL_H

#include <Sosage/Utils/error.h>
#include <Sosage/Config.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>

#include <array>
#include <sstream>

namespace Sosage::Third_party
{

class SDL
{
public:

  struct Image
  {
    SDL_Surface* surface;
    SDL_Texture* texture;
    double scaling;

    Image (SDL_Surface* surface = nullptr, SDL_Texture* texture = nullptr, double scaling = 1.)
      : surface (surface), texture (texture), scaling (scaling)
    { }
  };

  typedef SDL_Surface* Surface;
  typedef TTF_Font* Font;
  
  static SDL_Window* m_window;
  static SDL_Renderer* m_renderer;

  SDL_Surface* m_cursor_surf;
  SDL_Cursor* m_cursor;

  
public:

  SDL (const std::string& game_name, int width, int height, bool fullscreen);
  ~SDL ();

  void set_cursor (const std::string& file_name);
    

  static Image create_rectangle (int w, int h, int r, int g, int b, int a)
  {
    Uint32 rmask, gmask, bmask, amask;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
    rmask = 0xff000000;
    gmask = 0x00ff0000;
    bmask = 0x0000ff00;
    amask = 0x000000ff;
#else
    rmask = 0x000000ff;
    gmask = 0x0000ff00;
    bmask = 0x00ff0000;
    amask = 0xff000000;
#endif

    SDL_Surface* surf= SDL_CreateRGBSurface (0, w, h, 32, rmask, gmask, bmask, amask);
    check (surf != nullptr, "Cannot create rectangle surface");

    SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, r, g, b, a));

    SDL_Texture* out = SDL_CreateTextureFromSurface (m_renderer, surf);
    check (out != nullptr, "Cannot create rectangle texture");
    if (a != 255)
      SDL_SetTextureBlendMode(out, SDL_BLENDMODE_BLEND);
    
    return Image (surf, out, 1);
  }
  
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
    return Image (surf, out, 1);
  }

  static Font load_font (const std::string& file_name, int size)
  {
    TTF_Font* out = TTF_OpenFont (file_name.c_str(), size);
    check (out != nullptr, "Cannot load font " + file_name);
    return out;
  }

  static SDL_Color black()
  {
    SDL_Color out;
    out.r = 0;
    out.g = 0;
    out.b = 0;
    out.a = 0;
    return out;
  }
  
  static SDL_Color color (const std::string& color_str)
  {
    std::stringstream ss(color_str);
    int num;
    ss >> std::hex >> num;
    
    SDL_Color out;
    out.r = num / 0x10000;
    out.g = (num / 0x100) % 0x100;
    out.b = num % 0x100;
    return out;
  }
  
  static Image create_text (const Font& font, const std::string& color_str,
                            const std::string& text)
  {
    SDL_Surface* surf;
    if (text.find('\n') == std::string::npos)
      surf = TTF_RenderUTF8_Blended (font, text.c_str(), color(color_str));
    else
      surf = TTF_RenderUTF8_Blended_Wrapped (font, text.c_str(), color(color_str), 1920);
    
    check (surf != nullptr, "Cannot create text \"" + text + "\"");
    SDL_Texture* out = SDL_CreateTextureFromSurface (m_renderer, surf);
    check (out != nullptr, "Cannot create texture from text \"" + text + "\"");
    return Image (surf, out, 1);
  }

  static void rescale (Image& source, double scaling)
  {
    source.scaling = scaling;
  }

  static void delete_image (const Image& source)
  {
    SDL_FreeSurface (source.surface);
    SDL_DestroyTexture (source.texture);
  }

  static void delete_font (const Font& font)
  {
    TTF_CloseFont(font);
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
  
  static bool is_inside_image (Image image, int x, int y)
  {
    SDL_LockSurface (image.surface);

    int bpp = image.surface->format->BytesPerPixel;
    Uint8 *p = (Uint8 *)image.surface->pixels + y * image.surface->pitch + x * bpp;
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
    unsigned char r, g, b, a;
    SDL_GetRGBA(data, image.surface->format, &r, &g, &b, &a);
    SDL_UnlockSurface (image.surface);
    return (a != 0);
  }
  
  static int width (Surface image) { return image->w; }
  static int height (Surface image) { return image->h; }
  static int width (Image image)
  {
    int out;
    SDL_QueryTexture (image.texture, NULL, NULL, &out, NULL);
    return out;
  }
  static int height (Image image)
  {
    int out;
    SDL_QueryTexture (image.texture, NULL, NULL, NULL, &out);
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
