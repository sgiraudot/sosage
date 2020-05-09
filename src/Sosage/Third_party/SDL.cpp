/*
  [src/Sosage/Third_party/SDL.cpp]
  Wrapper for SDL library (images, render, fonts, etc.).

  =====================================================================

  This file is part of SOSAGE.

  SOSAGE is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  SOSAGE is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with SOSAGE.  If not, see <https://www.gnu.org/licenses/>.

  =====================================================================

  Author(s): Simon Giraudot <sosage@ptilouk.net>
*/

#include <Sosage/Third_party/SDL.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/error.h>
#include <Sosage/platform.h>

namespace Sosage::Third_party
{

SDL_Window* SDL::m_window = nullptr;
SDL_Renderer* SDL::m_renderer = nullptr;

SDL::Image SDL::create_rectangle (int w, int h, int r, int g, int b, int a)
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
  
SDL::Surface SDL::load_surface (const std::string& file_name)
{
  SDL_Surface* surf = IMG_Load (file_name.c_str());
  check (surf != nullptr, "Cannot load image " + file_name);
  return surf;
}

SDL::Image SDL::load_image (const std::string& file_name)
{
  SDL_Surface* surf = load_surface (file_name);
  SDL_Texture* out = SDL_CreateTextureFromSurface (m_renderer, surf);
  check (out != nullptr, "Cannot create texture from " + file_name);
  return Image (surf, out, 1);
}

SDL::Font SDL::load_font (const std::string& file_name, int size)
{
  TTF_Font* out = TTF_OpenFont (file_name.c_str(), size);
  check (out != nullptr, "Cannot load font " + file_name);
  TTF_Font* out2 = TTF_OpenFont (file_name.c_str(), size);
  check (out2 != nullptr, "Cannot load font " + file_name);
  TTF_SetFontOutline(out2, Sosage::text_outline);
  
  return std::make_pair(out, out2);
}

SDL_Color SDL::black()
{
  SDL_Color out;
  out.r = 0;
  out.g = 0;
  out.b = 0;
  out.a = 0;
  return out;
}
  
SDL_Color SDL::color (const std::string& color_str)
{
  std::stringstream ss(color_str);
  int num;
  ss >> std::hex >> num;

  std::array<unsigned char, 3> color
    = color_from_string (color_str);
  SDL_Color out;
  out.r = color[0];
  out.g = color[1];
  out.b = color[2];
  return out;
}
  
SDL::Image SDL::create_text (const SDL::Font& font, const std::string& color_str,
                             const std::string& text)
{
  SDL_Surface* surf;
  if (text.find('\n') == std::string::npos)
    surf = TTF_RenderUTF8_Blended (font.first, text.c_str(), color(color_str));
  else
    surf = TTF_RenderUTF8_Blended_Wrapped (font.first, text.c_str(), color(color_str), 1920);
    
  check (surf != nullptr, "Cannot create text \"" + text + "\"");
  SDL_Texture* out = SDL_CreateTextureFromSurface (m_renderer, surf);
  check (out != nullptr, "Cannot create texture from text \"" + text + "\"");
  return Image (surf, out, 1);
}

SDL::Image SDL::create_outlined_text (const SDL::Font& font, const std::string& color_str,
                                      const std::string& text)
{
  SDL_Surface* surf
    = TTF_RenderUTF8_Blended (font.first, text.c_str(), color(color_str));
  check (surf != nullptr, "Cannot create text \"" + text + "\"");
  
  SDL_Surface* back
    = TTF_RenderUTF8_Blended (font.second, text.c_str(), black());
  check (back != nullptr, "Cannot create text \"" + text + "\"");
  
  SDL_Rect rect = {Sosage::text_outline, Sosage::text_outline, surf->w, surf->h};
  SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND); 
  SDL_BlitSurface(surf, NULL, back, &rect);

  SDL_FreeSurface(surf);
  
  SDL_Texture* out = SDL_CreateTextureFromSurface (m_renderer, back);
  check (out != nullptr, "Cannot create texture from text \"" + text + "\"");
  return Image (back, out, 1);
}

void SDL::rescale (SDL::Image& source, double scaling)
{
  source.scaling = scaling;
}

void SDL::delete_image (const SDL::Image& source)
{
  SDL_FreeSurface (source.surface);
  SDL_DestroyTexture (source.texture);
}

void SDL::delete_font (const SDL::Font& font)
{
  TTF_CloseFont(font.first);
  TTF_CloseFont(font.second);
}

std::array<unsigned char, 3> SDL::get_color (SDL::Surface image, int x, int y)
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
  
bool SDL::is_inside_image (SDL::Image image, int x, int y)
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
  
int SDL::width (SDL::Surface image) { return image->w; }
int SDL::height (SDL::Surface image) { return image->h; }
int SDL::width (SDL::Image image)
{
  int out;
  SDL_QueryTexture (image.texture, NULL, NULL, &out, NULL);
  return out;
}
int SDL::height (SDL::Image image)
{
  int out;
  SDL_QueryTexture (image.texture, NULL, NULL, NULL, &out);
  return out;
}

SDL::SDL (const std::string& game_name)
  : m_cursor_surf (nullptr)
{
  int init = SDL_Init(SDL_INIT_VIDEO);
  check (init != -1, "Cannot initialize SDL");

  init = IMG_Init(IMG_INIT_PNG);
  check (init != -1, "Cannot initialize SDL Image");

  init = TTF_Init();
  check (init != -1, "Cannot initialize SDL TTF");

  std::cerr << config().window_width << " * " << config().window_height << std::endl;

  m_window = SDL_CreateWindow (game_name.c_str(),
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               config().window_width, config().window_height,
                               SDL_WINDOW_RESIZABLE |
                               (config().fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
  check (m_window != nullptr, "Cannot create SDL Window");
  
  m_renderer = SDL_CreateRenderer (m_window, -1, 0);
  check (m_renderer != nullptr, "Cannot create SDL Renderer");

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");
  
  SDL_GetWindowSize (m_window, &(config().window_width), &(config().window_height));

  update_view();

  // Render black screen while the rest is loading
  SDL_SetRenderDrawColor (m_renderer, 0, 0, 0, 255);
  SDL_RenderClear (m_renderer);
  SDL_RenderPresent (m_renderer);
  SDL_ShowCursor(SDL_DISABLE);
}

SDL::~SDL ()
{
  TTF_Quit ();
  IMG_Quit ();
  
  SDL_DestroyRenderer (m_renderer);
  SDL_DestroyWindow (m_window);
  if (m_cursor_surf != nullptr)
  {
    SDL_FreeSurface (m_cursor_surf);
    SDL_FreeCursor (m_cursor);
  }
  SDL_Quit ();
}

void SDL::update_view()
{
  int window_width = Sosage::world_width + config().interface_width;
  int window_height = Sosage::world_height + config().interface_height;

  SDL_RenderSetLogicalSize(m_renderer, window_width, window_height);
}

void SDL::begin ()
{
  SDL_SetRenderDrawColor (m_renderer, 0, 0, 0, 255);
  SDL_RenderClear (m_renderer);
}

void SDL::draw (const Image& image,
                const int xsource, const int ysource,
                const int wsource, const int hsource,
                const int xtarget, const int ytarget,
                const int wtarget, const int htarget)
{
  SDL_Rect source;
  source.x = xsource;
  source.y = ysource;
  source.w = wsource;
  source.h = hsource;
  
  SDL_Rect target;
  target.x = xtarget;
  target.y = ytarget;
  target.w = wtarget;
  target.h = htarget;
  
  SDL_RenderCopy(m_renderer, image.texture, &source, &target);
}

void SDL::draw_line (const int xa, const int ya, const int xb, const int yb,
                     unsigned int red, unsigned green, unsigned blue)
{
  SDL_SetRenderDrawColor(m_renderer, red, green, blue, 255);
  SDL_RenderDrawLine (m_renderer, xa, ya, xb, yb);
}

void SDL::draw_square (const int x, const int y, const int size,
                     unsigned int red, unsigned green, unsigned blue)
{
  SDL_Rect rect;
  rect.x = x - size / 2;
  rect.y = y - size / 2;
  rect.w = size;
  rect.h = size;

  SDL_SetRenderDrawColor(m_renderer, red, green, blue, 255);
  SDL_RenderFillRect(m_renderer, &rect);
}

void SDL::end ()
{
  SDL_RenderPresent (m_renderer);
}

} // namespace Sosage::Third_party

