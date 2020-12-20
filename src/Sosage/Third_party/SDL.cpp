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

#include <Sosage/Config/platform.h>
#include <Sosage/Third_party/SDL.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/error.h>

#include <functional>

namespace Sosage::Third_party
{

SDL_Window* SDL::m_window = nullptr;
SDL_Renderer* SDL::m_renderer = nullptr;
SDL_RendererInfo SDL::m_info;
SDL::Texture_manager SDL::m_textures (SDL_DestroyTexture);
SDL::Bitmap_manager SDL::m_masks;
SDL::Font_manager SDL::m_fonts (TTF_CloseFont);

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

  SDL_Surface* surf = SDL_CreateRGBSurface (0, w, h, 32, rmask, gmask, bmask, amask);
  check (surf != nullptr, "Cannot create rectangle surface ("
         + std::string(SDL_GetError()) + ")");

  SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, Uint8(r), Uint8(g), Uint8(b), Uint8(255)));

  Texture text = m_textures.make_single (SDL_CreateTextureFromSurface, m_renderer, surf);
  check (text != Texture(), "Cannot create rectangle texture ("
         + std::string(SDL_GetError()) + ")");
  Image out (text, Bitmap(), surf->w, surf->h, 1, (unsigned char)(a));
  SDL_FreeSurface(surf);
  return out;
}

SDL::Image SDL::load_image (const std::string& file_name, bool with_mask)
{
  SDL_Surface* surf = IMG_Load (file_name.c_str());
  check (surf != nullptr, "Cannot load image " + file_name
         + " (" + std::string(SDL_GetError()) + ")");

  int height = surf->h;
  int width = surf->w;
  int width_max = m_info.max_texture_width;
  int height_max = m_info.max_texture_height;
  double texture_downscale = 1.;
  if (surf->w > width_max || surf->h > height_max)
  {
    texture_downscale = std::min (double(width_max) / surf->w,
                                  double(height_max) / surf->h);
    debug (file_name, " is too large and will be downscaled by a factor ",
           texture_downscale);
    SDL_Surface* old = surf;
    surf = SDL_CreateRGBSurface
           (old->flags, old->w, old->h,
            32, old->format->Rmask, old->format->Gmask, old->format->Bmask, old->format->Amask);
    SDL_BlitSurface (old, nullptr, surf, nullptr);
    SDL_FreeSurface (old);

    old = surf;
    surf = SDL_CreateRGBSurface
           (old->flags, int(texture_downscale * old->w), int(texture_downscale * old->h),
            32, old->format->Rmask, old->format->Gmask, old->format->Bmask, old->format->Amask);
    check (surf != nullptr, "Cannot create rectangle surface ("
           + std::string(SDL_GetError()) + ")");

    int result = SDL_BlitScaled(old, nullptr, surf, nullptr);
    check (result == 0, "Couldn't blit surface ("
           + std::string(SDL_GetError()) + ")");
    SDL_FreeSurface(old);
  }


  Texture text = m_textures.make_mapped (file_name, SDL_CreateTextureFromSurface, m_renderer, surf);
  check (text != Texture(), "Cannot create texture from " + file_name
         + " (" + std::string(SDL_GetError()) + ")");
  Bitmap mask = nullptr;
  if (with_mask)
    mask = m_masks.make_mapped (file_name, create_mask, surf);
  Image out (text, mask, width, height, 1);
  out.texture_downscale = texture_downscale;
  SDL_FreeSurface(surf);
  return out;
}

SDL::Font SDL::load_font (const std::string& file_name, int size)
{
  Font_base out = m_fonts.make_mapped (file_name, TTF_OpenFont, file_name.c_str(), size);
  check (out != Font_base(), "Cannot load font " + file_name);
  Font_base out2 = m_fonts.make_mapped (file_name + ".outlined", TTF_OpenFont, file_name.c_str(), size);
  check (out2 != Font_base(), "Cannot load font " + file_name);
  TTF_SetFontOutline(out2.get(), Config::text_outline);

  return std::make_pair(out, out2);
}

Bitmap_2* SDL::create_mask (SDL_Surface* surf)
{
  Bitmap_2* out = new Bitmap_2 (surf->w, surf->h);
  SDL_LockSurface (surf);

  int bpp = surf->format->BytesPerPixel;

  for (std::size_t x = 0; x < out->width(); ++ x)
    for (std::size_t y = 0; y < out->height(); ++ y)
    {
      Uint8 *p = (Uint8 *)surf->pixels + y * surf->pitch + x * bpp;
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
            data = Uint32(p[0] << 16 | p[1] << 8 | p[2]);
          else
            data = Uint32(p[0] | p[1] << 8 | p[2] << 16);
          break;

        case 4:
          data = *(Uint32 *)p;
          break;

        default:
          exit(0);
      }
      unsigned char r, g, b, a;
      SDL_GetRGBA(data, surf->format, &r, &g, &b, &a);
      (*out)(x, y) = (a != 0);
    }

  SDL_UnlockSurface (surf);

  return out;
}


SDL_Color SDL::black()
{
  SDL_Color out;
  out.r = 0;
  out.g = 0;
  out.b = 0;
  out.a = 255;
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
  out.a = 255;
  return out;
}

SDL::Image SDL::create_text (const SDL::Font& font, const std::string& color_str,
                             const std::string& text)
{
  SDL_Surface* surf;
  if (text.find('\n') == std::string::npos)
    surf = TTF_RenderUTF8_Blended(font.first.get(), text.c_str(), color(color_str));
  else
    surf = TTF_RenderUTF8_Blended_Wrapped(font.first.get(), text.c_str(), color(color_str), 1920);

  check (surf != nullptr, "Cannot create text \"" + text + "\"");
  Texture texture = m_textures.make_single (SDL_CreateTextureFromSurface, m_renderer, surf);
  check (texture != Texture(), "Cannot create texture from text \"" + text + "\"");
  Image out (texture, Bitmap(), surf->w, surf->h, 1);
  SDL_FreeSurface (surf);
  return out;
}

SDL::Image SDL::create_outlined_text (const SDL::Font& font, const std::string& color_str,
                                      const std::string& text)
{
  SDL_Surface* surf = TTF_RenderUTF8_Blended (font.first.get(), text.c_str(), color(color_str));
  check (surf != nullptr, "Cannot create text \"" + text + "\"");

  SDL_Surface* back = TTF_RenderUTF8_Blended (font.second.get(), text.c_str(), black());
  check (back != nullptr, "Cannot create text \"" + text + "\"");

  SDL_Rect rect = {Config::text_outline, Config::text_outline, surf->w, surf->h};
  SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
  SDL_BlitSurface (surf, NULL, back, &rect);

  Texture texture = m_textures.make_single (SDL_CreateTextureFromSurface, m_renderer, back);
  check (texture != Texture(), "Cannot create texture from text \"" + text + "\"");
  Image out (texture, Bitmap(), back->w, back->h, 1);
  SDL_FreeSurface (surf);
  SDL_FreeSurface (back);
  return out;
}

void SDL::rescale (SDL::Image& source, double scaling)
{
  source.scaling = scaling;
}

int SDL::width (SDL::Image image)
{
  return image.width;
}
int SDL::height (SDL::Image image)
{
  return image.height;
}


SDL::Surface SDL::load_surface (const std::string& file_name)
{
  Surface surf (IMG_Load(file_name.c_str()), SDL_FreeSurface);
  check (surf != Surface(), "Cannot load image " + file_name);
  return surf;
}

std::array<unsigned char, 3> SDL::get_color (SDL::Surface image, int x, int y)
{
  SDL_LockSurface (image.get());

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
        data = Uint32(p[0] << 16 | p[1] << 8 | p[2]);
      else
        data = Uint32(p[0] | p[1] << 8 | p[2] << 16);
      break;

    case 4:
      data = *(Uint32 *)p;
      break;

    default:
      exit(0);
  }
  std::array<unsigned char, 3> out;
  SDL_GetRGB(data, image->format, &out[0], &out[1], &out[2]);
  SDL_UnlockSurface (image.get());
  return out;
}


SDL::SDL ()
{

}

void SDL::init (int& window_width, int& window_height, bool fullscreen)
{
  int okay = SDL_Init(SDL_INIT_VIDEO);
  check (okay != -1, "Cannot initialize SDL");

  okay = IMG_Init(IMG_INIT_PNG);
  check (okay != -1, "Cannot initialize SDL Image");

  okay = TTF_Init();
  check (okay != -1, "Cannot initialize SDL TTF");

  if (window_width == -1 || window_height == -1)
  {
    SDL_DisplayMode DM;
    SDL_GetCurrentDisplayMode(0, &DM);
    window_height = int(0.8 * DM.h);
    window_width = int(1.6 * window_height);
  }
  m_window = SDL_CreateWindow ("",
                               SDL_WINDOWPOS_CENTERED,
                               SDL_WINDOWPOS_CENTERED,
                               window_width, window_height,
                               SDL_WINDOW_RESIZABLE |
                               (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
  check (m_window != nullptr, "Cannot create SDL Window ("
         + std::string(SDL_GetError()) + ")");

  m_renderer = SDL_CreateRenderer (m_window, -1, 0);
  check (m_renderer != nullptr, "Cannot create SDL Renderer");

  int result = SDL_GetRendererInfo (m_renderer, &m_info);
  check (result == 0, "Cannot create SDL Renderer Info");

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

  // Render black screen while the rest is loading
  SDL_SetRenderDrawColor (m_renderer, 0, 0, 0, 255);
  SDL_RenderClear (m_renderer);
  SDL_RenderPresent (m_renderer);
  SDL_ShowCursor(SDL_DISABLE);
}

SDL::~SDL ()
{
  m_textures.clear();
  m_masks.clear();
  m_fonts.clear();
  SDL_FreeSurface (m_icon);
  TTF_Quit ();
  IMG_Quit ();
  SDL_DestroyRenderer (m_renderer);
  SDL_DestroyWindow (m_window);
  SDL_Quit ();
}

void SDL::update_window (const std::string& name, const std::string& icon_filename)
{
  SDL_SetWindowTitle (m_window, name.c_str());
  m_icon = IMG_Load(icon_filename.c_str());
  SDL_SetWindowIcon (m_window, m_icon);
}

void SDL::update_view(int interface_width, int interface_height)
{
  int window_width = Config::world_width + interface_width;
  int window_height = Config::world_height + interface_height;

  SDL_RenderSetLogicalSize(m_renderer, window_width, window_height);
}

void SDL::toggle_fullscreen (bool fullscreen)
{
  SDL_SetWindowFullscreen (m_window, (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));
}

void SDL::begin (int interface_width, int interface_height)
{
  // Out of bound background is gray
  SDL_SetRenderDrawColor (m_renderer, 48, 48, 48, 255);
  SDL_RenderClear (m_renderer);

  // Inside background is black
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = Config::world_width + interface_width;
  rect.h = Config::world_height + interface_height;

  SDL_SetRenderDrawColor(m_renderer, Uint8(0), Uint8(0), Uint8(0), 255);
  SDL_RenderFillRect(m_renderer, &rect);
}

void SDL::draw (const Image& image,
                const int xsource, const int ysource,
                const int wsource, const int hsource,
                const int xtarget, const int ytarget,
                const int wtarget, const int htarget)
{
  SDL_Rect source;
  source.x = image.texture_downscale * xsource;
  source.y = image.texture_downscale * ysource;
  source.w = image.texture_downscale * wsource;
  source.h = image.texture_downscale * hsource;

  SDL_Rect target;
  target.x = xtarget;
  target.y = ytarget;
  target.w = wtarget;
  target.h = htarget;

  SDL_SetTextureAlphaMod(image.texture.get(), image.alpha);
  SDL_RenderCopy(m_renderer, image.texture.get(), &source, &target);
}

void SDL::draw_line (const int xa, const int ya, const int xb, const int yb,
                     unsigned int red, unsigned green, unsigned blue)
{
  SDL_SetRenderDrawColor(m_renderer, Uint8(red), Uint8(green), Uint8(blue), 255);
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

  SDL_SetRenderDrawColor(m_renderer, Uint8(red), Uint8(green), Uint8(blue), 255);
  SDL_RenderFillRect(m_renderer, &rect);
}

void SDL::end ()
{
  SDL_RenderPresent (m_renderer);
}

} // namespace Sosage::Third_party
