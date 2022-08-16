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

#include <Sosage/Config/config.h>
#include <Sosage/Third_party/SDL.h>
#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/geometry.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Utils/image_split.h>
#include <Sosage/Utils/profiling.h>

#include <SDL_image.h>

#include <sstream>

namespace Sosage::Third_party
{

SDL_Window* SDL::m_window = nullptr;
SDL_Renderer* SDL::m_renderer = nullptr;
SDL::Image_manager SDL::m_images
([](Image_base* img)
{
  for (SDL_Texture* t : img->texture)
    if (t != nullptr)
      SDL_DestroyTexture (t);
  for (SDL_Texture* t : img->highlight)
    if (t != nullptr)
      SDL_DestroyTexture (t);
  delete img;
});
SDL::Font_manager SDL::m_fonts
([](Font_base* font)
{
  TTF_CloseFont(std::get<0>(*font));
  TTF_CloseFont(std::get<1>(*font));
  delete std::get<2>(*font);
  delete font;
});
void* SDL::m_buffer = nullptr;
void* SDL::m_hbuffer = nullptr;

SDL::Image_base* SDL::make_images (const std::vector<SDL_Texture*>& texture,
                                   const std::vector<SDL_Texture*>& highlight,
                                   int width, int height)
{
  Image_base* out = new Image_base();
  out->texture = texture;
  out->highlight = highlight;
  out->width = width;
  out->height = height;
  return out;
}

SDL::Image_base* SDL::make_image (SDL_Texture* texture,
                                  SDL_Texture* highlight,
                                  int width, int height)
{
  Image_base* out = new Image_base();
  out->texture.push_back(texture);
  out->highlight.push_back(highlight);
  out->width = width;
  out->height = height;
  return out;
}

SDL::Surface_access::Surface_access (SDL_Surface* surface)
  : surface (surface)
{
  if (SDL_MUSTLOCK(surface))
    SDL_LockSurface(surface);
  bpp = surface->format->BytesPerPixel;
}

std::size_t SDL::Surface_access::width()
{
  return surface->w;
}

std::size_t SDL::Surface_access::height()
{
  return surface->h;
}

RGBA_color SDL::Surface_access::get (std::size_t x, std::size_t y) const
{
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
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
  SDL_GetRGBA(data, surface->format, &r, &g, &b, &a);
  return { r, g, b, a };
}

void SDL::Surface_access::set (std::size_t x, std::size_t y, const RGBA_color& color) const
{
  Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
  Uint32 data = SDL_MapRGBA(surface->format, color[0], color[1], color[2], color[3]);

  switch (bpp)
  {
    case 1:
      *p = data;
      break;

    case 2:
      *(Uint16 *)p = data;
      break;

    case 3:
      if(SDL_BYTEORDER == SDL_BIG_ENDIAN)
      {
        p[0] = (data>> 16) & 0xff;
        p[1] = (data >> 8) & 0xff;
        p[2] = data & 0xff;
      }
      else
      {
        p[0] = data & 0xff;
        p[1] = (data >> 8) & 0xff;
        p[2] = (data >> 16) & 0xff;
      }
      break;

    case 4:
      *(Uint32 *)p = data;
      break;
  }
}

void SDL::Surface_access::release()
{
  if (SDL_MUSTLOCK(surface))
    SDL_UnlockSurface(surface);
}

std::pair<SDL::Image, double> SDL::create_rectangle (int w, int h, int r, int g, int b, int a)
{
  SOSAGE_TIMER_START(SDL_Image__create_rectangle);

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

  double scale = 1.;
  if (h*w > int(Splitter::max_length * Splitter::max_length))
  {
    scale = std::ceil(h*w / double(Splitter::max_length * Splitter::max_length));

    if (w != round(int(w / scale) * scale) || h != round(int(h / scale) * scale))
    {
      debug << "Warning: scaled rectangle's size is modified: " << w << "x" << h << " -> " << round(int(w / scale) * scale) << "x" << round(int(h / scale) * scale) << std::endl;
    }

    w /= scale;
    h /= scale;
  }

  SDL_Surface* surf = SDL_CreateRGBSurfaceFrom (m_buffer, w, h, 32, w * 4, rmask, gmask, bmask, amask);
  check (surf != nullptr, "Cannot create rectangle surface ("
         + std::string(SDL_GetError()) + ")");

  SDL_FillRect(surf, nullptr, SDL_MapRGBA(surf->format, Uint8(r), Uint8(g), Uint8(b), Uint8(a)));

#ifndef SOSAGE_GUILESS
  SDL_Texture* text = SDL_CreateTextureFromSurface (m_renderer, surf);
  check (text != nullptr, "Cannot create rectangle texture ("
         + std::string(SDL_GetError()) + ")");
#else
  SDL_Texture* text;
#endif

  SDL_Texture* highlight = nullptr;
#ifndef SOSAGE_GUILESS
  if (a == 0) // special ellipse highlight for fully transparent objects
  {
    SDL_Surface* high = SDL_CreateRGBSurfaceFrom (m_hbuffer, surf->w,
                                                  surf->h, 32, surf->w * 4, rmask, gmask, bmask, amask);

    Surface_access access(high);
    for (std::size_t i = 0; i < access.width(); ++ i)
      for (std::size_t j = 0; j < access.height(); ++ j)
      {
        double x = std::abs(double(i) - 0.5 * access.width()) / double(0.5 * access.width());
        double y = std::abs(double(j) - 0.5 * access.height()) / double(0.5 * access.height());
        double dist_to_center = x*x + y*y;
        if (dist_to_center > 1.0)
          access.set(i,j, { 255, 255, 255, 0 });
        else
          access.set(i,j, { 255, 255, 255,
                            (unsigned char)(255 * (1.0 - dist_to_center)) });
      }

    access.release();
    highlight = SDL_CreateTextureFromSurface (m_renderer, high);
    SDL_FreeSurface (high);
  }
#endif

  SDL_FreeSurface(surf);

  SOSAGE_TIMER_STOP(SDL_Image__create_rectangle);

  return std::make_pair (m_images.make_single (make_image, text, highlight, w, h),
                         scale);
}

SDL::Image SDL::load_image (const std::string& file_name, bool with_mask, bool with_highlight)
{
  SOSAGE_TIMER_START(SDL_Image__load_image);

  Image out = m_images.make_mapped
      (file_name,
       [&]() -> Image_base*
       {
         Uint32 nb_x = 1;
         Uint32 nb_y = 1;
         int width = -1, height = -1;
         int format_int = -1;
         Bitmap_2 mask;
         std::vector<SDL_Texture*> textures;
         std::vector<SDL_Texture*> highlights;
         if (Asset_manager::packaged())
         {
           std::tie (width, height, format_int) = Asset_manager::image_info (file_name);
           nb_x = Splitter::nb_sub (width);
           nb_y = Splitter::nb_sub (height);
           textures.reserve(nb_x * nb_y);
           highlights.reserve(nb_x * nb_y);
         }

         for (Uint32 x = 0; x < nb_x; ++ x)
         {
           for (Uint32 y = 0; y < nb_y; ++ y)
           {
             SDL_Surface* surf = nullptr;
             SDL_Texture* highlight = nullptr;
             SDL_Texture* texture = nullptr;
             if (Asset_manager::packaged())
             {
               SDL_Rect rect = Splitter::rect (width, height, x, y);
               SOSAGE_TIMER_START(SDL_Image__load_image_file);
               surf = SDL_CreateRGBSurfaceWithFormatFrom (m_buffer, rect.w, rect.h, 32, rect.w * 4, format_int);
               SDL_LockSurface (surf);
               Asset_manager::open (file_name, surf->pixels, x, y);
               SDL_UnlockSurface (surf);
               SOSAGE_TIMER_STOP(SDL_Image__load_image_file);

#ifndef SOSAGE_GUILESS
               SOSAGE_TIMER_START(SDL_Image__load_image_texture);
               texture = SDL_CreateTextureFromSurface(m_renderer, surf);
               check (texture != nullptr, "Cannot create texture from " + file_name
               + " (" + std::string(SDL_GetError()) + ")");
               SOSAGE_TIMER_STOP(SDL_Image__load_image_texture);

               if (with_highlight)
               {
                 SOSAGE_TIMER_START(SDL_Image__load_image_create_highlight);
                 SDL_Surface* high = nullptr;
                 high = SDL_CreateRGBSurfaceWithFormatFrom (m_hbuffer, rect.w, rect.h, 32, rect.w * 4, format_int);
                 SDL_LockSurface (high);
                 Asset_manager::open (file_name, high->pixels, x, y, true);
                 SDL_UnlockSurface (high);
                 SOSAGE_TIMER_STOP(SDL_Image__load_image_create_highlight);

                 SOSAGE_TIMER_START(SDL_Image__load_image_hightlight_2);
                 highlight = SDL_CreateTextureFromSurface(m_renderer, high);
                 SDL_FreeSurface (high);
                 SOSAGE_TIMER_STOP(SDL_Image__load_image_hightlight_2);
               }
#endif
             }
             else
             {
               SOSAGE_TIMER_START(SDL_Image__load_image_file);
               Asset asset = Asset_manager::open(file_name);
               surf = IMG_Load_RW (asset.base(), 1);
               check (surf != nullptr, "Cannot load image " + file_name
               + " (" + std::string(SDL_GetError()) + ")");
               SOSAGE_TIMER_STOP(SDL_Image__load_image_file);
               height = surf->h;
               width = surf->w;
#ifndef SOSAGE_GUILESS
               SOSAGE_TIMER_START(SDL_Image__load_image_texture);
               texture = SDL_CreateTextureFromSurface(m_renderer, surf);
               check (texture != nullptr, "Cannot create texture from " + file_name
               + " (" + std::string(SDL_GetError()) + ")");
               SOSAGE_TIMER_STOP(SDL_Image__load_image_texture);

               if (with_highlight)
               {
                 SOSAGE_TIMER_START(SDL_Image__load_image_create_highlight);
                 SDL_Surface* high = SDL_CreateRGBSurfaceWithFormat (0, surf->w, surf->h, 32, SDL_PIXELFORMAT_ARGB8888);
                 SDL_FillRect(high, nullptr, SDL_MapRGBA(high->format, Uint8(0), Uint8(0), Uint8(0), Uint8(0)));
                 SDL_BlitSurface (surf, nullptr, high, nullptr);

                 Surface_access access (high);
                 for (std::size_t j = 0; j < access.height(); ++ j)
                 for (std::size_t i = 0; i < access.width(); ++ i)
                 access.set(i,j, {255, 255, 255, (unsigned char)(0.5 * access.get(i,j)[3])});
                 access.release();
                 SOSAGE_TIMER_STOP(SDL_Image__load_image_create_highlight);

                 SOSAGE_TIMER_START(SDL_Image__load_image_hightlight_2);
                 highlight = SDL_CreateTextureFromSurface(m_renderer, high);
                 SDL_FreeSurface (high);
                 SOSAGE_TIMER_STOP(SDL_Image__load_image_hightlight_2);
               }
#endif
             }

             textures.push_back(texture);
             highlights.push_back(highlight);

             if (with_mask)
             {
               SOSAGE_TIMER_START(SDL_Image__load_image_mask);
               if (Asset_manager::packaged())
               {
                 mask = Bitmap_2 (width, height, false);
                 Asset asset = Asset_manager::open (file_name + ".mask");
                 asset.read (mask.data(), mask.size());
                 asset.close();
               }
               else
                 mask = create_mask(surf);
               SOSAGE_TIMER_STOP(SDL_Image__load_image_mask);
             }
             SDL_FreeSurface(surf);
           }
         }

         auto out = make_images (textures, highlights, width, height);
         if (with_mask)
           out->mask = mask;
         return out;
       });

  SOSAGE_TIMER_STOP(SDL_Image__load_image);

  return out;
}

SDL::Image SDL::compose (const std::initializer_list<SDL::Image>& images)
{
  // Compose images horitonzally
  Uint32 total_height = 0;
  Uint32 total_width = 0;
  for (SDL::Image img : images)
  {
    total_height = std::max(Uint32(height(img)), total_height);
    total_width += width (img);
  }

#ifndef SOSAGE_GUILESS
  SDL_Texture* texture = SDL_CreateTexture (m_renderer, SDL_PIXELFORMAT_ARGB8888,
                                            SDL_TEXTUREACCESS_TARGET, total_width, total_height);

  SDL_SetRenderTarget(m_renderer, texture);
  SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
  SDL_RenderClear(m_renderer);
  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetTextureBlendMode (texture, SDL_BLENDMODE_BLEND);
  Uint32 x = 0;
  for (SDL::Image img : images)
  {
    SDL_Rect rect;
    rect.h = height (img);
    rect.w = width (img);
    rect.x = x;
    rect.y = (total_height - rect.h) / 2;
    SDL_SetTextureBlendMode (img->texture[0], SDL_BLENDMODE_BLEND);
    SDL_RenderCopy (m_renderer, img->texture[0], nullptr, &rect);
    x += rect.w;
  }

  SDL_Texture* highlight = SDL_CreateTexture (m_renderer, SDL_PIXELFORMAT_ARGB8888,
                                              SDL_TEXTUREACCESS_TARGET, total_width, total_height);

  SDL_SetRenderTarget(m_renderer, highlight);
  SDL_SetRenderDrawColor(m_renderer, 0, 0, 0, 0);
  SDL_RenderClear(m_renderer);
  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SDL_SetTextureBlendMode (highlight, SDL_BLENDMODE_BLEND);
  x = 0;
  for (SDL::Image img : images)
  {
    SDL_Rect rect;
    rect.h = height (img);
    rect.w = width (img);
    rect.x = x;
    rect.y = (total_height - rect.h) / 2;
    if (img->highlight[0])
    {
      SDL_SetTextureBlendMode (img->highlight[0], SDL_BLENDMODE_BLEND);
      SDL_RenderCopy (m_renderer, img->highlight[0], nullptr, &rect);
    }
    x += rect.w;
  }

  SDL_SetRenderTarget(m_renderer, nullptr);
#else
  SDL_Texture* texture;
  SDL_Texture* highlight;
#endif

  return m_images.make_single (make_image, texture, highlight, total_width, total_height);
}

SDL::Font SDL::load_font (const std::string& file_name, int size)
{
  Asset asset = Asset_manager::open(file_name);
  Font out = m_fonts.make_mapped
    (file_name,
     [&]() -> Font_base*
     {
       TTF_Font* font = TTF_OpenFontRW(asset.base(), 0, size);
       check (font != nullptr, "Cannot load font " + file_name);
       asset.seek(0);
       TTF_Font* outlined = TTF_OpenFontRW(asset.base(), 1, size);
       check (outlined != nullptr, "Cannot load outlined font " + file_name);
       TTF_SetFontOutline (outlined, Config::text_outline);
       return new Font_base (font, outlined, asset.buffer());
     });
  return out;
}

Bitmap_2 SDL::create_mask (SDL_Surface* surf)
{
  Bitmap_2 out (surf->w, surf->h);

  Surface_access access (surf);

  for (std::size_t y = 0; y < out.height(); ++ y)
    for (std::size_t x = 0; x < out.width(); ++ x)
      out.set (x, y, access.get(x,y)[3] != 0);

  access.release();

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
  // Debug info constantly changes, so saving each version leads to a
  // memory explosion. This fix is a bit hacky...
  if (contains(text, "[Debug info]"))
  {
    SDL_Surface* surf;
    surf = TTF_RenderUTF8_Blended_Wrapped(std::get<0>(*font), text.c_str(), color(color_str), 1920);
    int width = surf->w;
    int height = surf->h;
    check (surf != nullptr, "Cannot create text \"" + text + "\"");
#ifndef SOSAGE_GUILESS
    SDL_Texture* out = SDL_CreateTextureFromSurface (m_renderer, surf);
    check (out != nullptr, "Cannot create texture from text \"" + text + "\"");
#else
    SDL_Texture* out = nullptr;
#endif
    SDL_FreeSurface (surf);
    return m_images.make_single(make_image, out, nullptr, width, height);
  }

  std::string id = to_string(std::size_t(std::get<0>(*font))) + color_str + text;
  return m_images.make_mapped
      (id, [&]() -> Image_base*
  {
    SDL_Surface* surf = nullptr;
    if (!contains (text, "\n"))
      surf = TTF_RenderUTF8_Blended(std::get<0>(*font), text.c_str(), color(color_str));
    else
      surf = TTF_RenderUTF8_Blended_Wrapped(std::get<0>(*font), text.c_str(), color(color_str), 1920);
    check (surf != nullptr, "Cannot create text \"" + text + "\"");

    int width = surf->w;
    int height = surf->h;

#ifndef SOSAGE_GUILESS
    SDL_Texture* out = SDL_CreateTextureFromSurface(m_renderer, surf);
    check (out != nullptr, "Cannot create texture from text \"" + text + "\"");
#else
    SDL_Texture* out = nullptr;
#endif
    SDL_FreeSurface (surf);
    return make_image (out, nullptr, width, height);
  });
}

SDL::Image SDL::create_outlined_text (const SDL::Font& font, const std::string& color_str,
                                      const std::string& text)
{
  std::string id = to_string(std::size_t(std::get<0>(*font))) + "outlined" + color_str + text;

  return m_images.make_mapped
    (id,
     [&]() -> Image_base*
     {
       SDL_Surface* surf = TTF_RenderUTF8_Blended (std::get<0>(*font), text.c_str(), color(color_str));
       check (surf != nullptr, "Cannot create text \"" + text + "\"");

       SDL_Surface* back = TTF_RenderUTF8_Blended (std::get<1>(*font), text.c_str(), black());
       check (back != nullptr, "Cannot create text \"" + text + "\"");

       SDL_Rect rect = {Config::text_outline, Config::text_outline, surf->w, surf->h};
       SDL_SetSurfaceBlendMode(surf, SDL_BLENDMODE_BLEND);
       SDL_BlitSurface (surf, NULL, back, &rect);

       int width = back->w;
       int height = back->h;

#ifndef SOSAGE_GUILESS
       SDL_Texture* out = SDL_CreateTextureFromSurface(m_renderer, back);
       check (out != nullptr, "Cannot create texture from text \"" + text + "\"");
#else
       SDL_Texture* out = nullptr;
#endif
       SDL_FreeSurface (surf);
       SDL_FreeSurface (back);
       return make_image (out, nullptr, width, height);
     });
}


int SDL::width (SDL::Image image)
{
  return image->width;
}
int SDL::height (SDL::Image image)
{
  return image->height;
}

SDL::Surface SDL::load_surface (const std::string& file_name)
{
  Surface surf;
  if (Asset_manager::packaged())
  {
    int width, height;
    int format_int;
    std::tie (width, height, format_int) = Asset_manager::image_info (file_name);

    surf = Surface(SDL_CreateRGBSurfaceWithFormat (0, width, height, 32, format_int), SDL_FreeSurface);
    SDL_LockSurface (surf.get());
    Asset_manager::open (file_name, surf->pixels);
    SDL_UnlockSurface (surf.get());
  }
  else
  {
    Asset asset = Asset_manager::open(file_name);
    surf = Surface(IMG_Load_RW(asset.base(), 1), SDL_FreeSurface);
  }
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

void SDL::display_error (const std::string& error)
{
  SDL_ShowSimpleMessageBox(0, "Error", error.c_str(), m_window);
}

SDL::SDL ()
{
  m_buffer = (void*)(new char[Splitter::max_length * Splitter::max_length * 4]);
  m_hbuffer = (void*)(new char[Splitter::max_length * Splitter::max_length * 4]);
}

void SDL::init (int& window_width, int& window_height, bool fullscreen)
{
#ifdef SOSAGE_GUILESS
  putenv("SDL_VIDEODRIVER=dummy");
#endif
  int okay = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_GAMECONTROLLER);
  check (okay != -1, "Cannot initialize SDL: (" + std::string(SDL_GetError()) + ")");

  okay = IMG_Init(IMG_INIT_PNG);
  check (okay != -1, "Cannot initialize SDL Image");

  okay = TTF_Init();
  check (okay != -1, "Cannot initialize SDL TTF");

#ifndef SOSAGE_GUILESS
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

  m_renderer = SDL_CreateRenderer (m_window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  check (m_renderer != nullptr, "Cannot create SDL Renderer: (" + std::string(SDL_GetError()) + ")");

  SDL_DisplayMode mode;
  SDL_GetWindowDisplayMode(m_window, &mode);
  debug << "Refresh rate: " << mode.refresh_rate << "Hz" << std::endl;

  SDL_RendererInfo info;
  int result = SDL_GetRendererInfo (m_renderer, &info);
  check (result == 0, "Cannot create SDL Renderer Info");

  debug << "Video display: " << SDL_GetCurrentVideoDriver() << std::endl;
  debug << "Available video displays: " << std::endl;
  for (int i = 0; i < SDL_GetNumVideoDrivers(); ++ i)
  {
    debug << " * " << SDL_GetVideoDriver(i) << std::endl;
  }

  debug << "Renderer name: " << info.name << std::endl;
  debug << "Supported texture formats: " << std::endl;
  for (Uint32 i = 0; i < info.num_texture_formats; ++ i)
  {
    debug << " * " << SDL_GetPixelFormatName (info.texture_formats[i]) << std::endl;
  }

  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "best");

  // Render black screen while the rest is loading
  SDL_SetRenderDrawColor (m_renderer, 0, 0, 0, 255);
  SDL_SetRenderDrawBlendMode(m_renderer, SDL_BLENDMODE_BLEND);
  SDL_RenderClear (m_renderer);
  SDL_RenderPresent (m_renderer);
  SDL_ShowCursor(SDL_DISABLE);
#endif

  SDL_GameController *controller = NULL;
  for (int i = 0; i < SDL_NumJoysticks(); ++i) {
      if (SDL_IsGameController(i)) {
          controller = SDL_GameControllerOpen(i);
          if (controller) {
              break;
          } else {
              fprintf(stderr, "Could not open gamecontroller %i: %s\n", i, SDL_GetError());
          }
      }
  }

}

SDL::~SDL ()
{
  delete[] (char*)m_buffer;
  delete[] (char*)m_hbuffer;
  clear_managers();
  TTF_Quit ();
  IMG_Quit ();
#ifndef SOSAGE_GUILESS
  SDL_DestroyRenderer (m_renderer);
  SDL_DestroyWindow (m_window);
#endif
  SDL_Quit ();
}

void SDL::clear_managers()
{
  m_images.clear();
  m_fonts.clear();
}

void SDL::update_window (const std::string& name, const std::string& icon_filename)
{
  SDL_SetWindowTitle (m_window, name.c_str());
  m_icon = load_surface(icon_filename);
  SDL_SetWindowIcon (m_window, m_icon.get());
}

void SDL::update_view()
{
  int width, height;
  SDL_GetWindowSize (m_window, &width, &height);
  double ratio_w = width / double(Config::world_width);
  double ratio_h = height / double(Config::world_height);
  double ratio = std::min(ratio_w, ratio_h);

  // To avoid fullscreen overlay to miss one pixel on the border
  // because of integer rounding,
  // we compute the "real" size (can be 1919 instead of 1920)
  width = int(int(Config::world_width * ratio) / ratio);
  height = int(int(Config::world_height * ratio) / ratio);
  SDL_RenderSetLogicalSize(m_renderer, width, height);
}

void SDL::toggle_fullscreen (bool fullscreen)
{
  SDL_SetWindowFullscreen (m_window, (fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0));

}

void SDL::toggle_cursor (bool visible)
{
  SDL_ShowCursor(visible ? SDL_ENABLE : SDL_DISABLE);
}

void SDL::begin()
{
#ifndef SOSAGE_GUILESS
  // Out of bound background is gray
  SDL_SetRenderDrawColor (m_renderer, 48, 48, 48, 255);
  SDL_RenderClear (m_renderer);

  // Inside background is black
  SDL_Rect rect;
  rect.x = 0;
  rect.y = 0;
  rect.w = Config::world_width;
  rect.h = Config::world_height;

  SDL_SetRenderDrawColor(m_renderer, Uint8(0), Uint8(0), Uint8(0), 255);
  SDL_RenderFillRect(m_renderer, &rect);
#endif
}

void SDL::draw (const Image& image, unsigned char alpha,
                unsigned char highlight_alpha,
                const int xsource, const int ysource,
                const int wsource, const int hsource,
                const double xtarget, const double ytarget,
                const double wtarget, const double htarget)
{
#ifndef SOSAGE_GUILESS
  if (image->texture.size() == 1)
  {
    SDL_Rect source;
    source.x = xsource;
    source.y = ysource;
    source.w = wsource;
    source.h = hsource;

    SDL_FRect target;
    target.x = xtarget;
    target.y = ytarget;
    target.w = wtarget;
    target.h = htarget;

    SDL_SetTextureAlphaMod(image->texture[0], alpha);
    SDL_RenderCopyF(m_renderer, image->texture[0], &source, &target);
    if (image->highlight[0] != nullptr && highlight_alpha != 0)
    {
      SDL_SetTextureAlphaMod(image->highlight[0], alpha);
      SDL_RenderCopyF(m_renderer, image->highlight[0], &source, &target);
    }
  }
  else
  {
    SDL_Rect source;
    source.x = xsource;
    source.y = ysource;
    source.w = wsource;
    source.h = hsource;

    double scale = htarget / hsource;

    Uint32 nb_x = Splitter::nb_sub (image->width);
    Uint32 nb_y = Splitter::nb_sub (image->height);

    Uint32 idx = 0;
    for (Uint32 x = 0; x < nb_x; ++ x)
    {
      for (Uint32 y = 0; y < nb_y; ++ y)
      {
        SDL_Rect rect = Splitter::rect (image->width, image->height, x, y);

        SDL_Rect inter;
        if (SDL_IntersectRect (&source, &rect, &inter) == SDL_FALSE)
        {
          ++ idx;
          continue;
        }

        SDL_FRect target;
        target.x = (inter.x - xsource) * scale + xtarget;
        target.y = (inter.y - ysource) * scale + ytarget;
        target.w = scale * inter.w;
        target.h = scale * inter.h;

        inter.x -= rect.x;
        inter.y -= rect.y;

        SDL_SetTextureAlphaMod(image->texture[idx], alpha);
        SDL_RenderCopyF(m_renderer, image->texture[idx], &inter, &target);
        if (image->highlight[idx] != nullptr && highlight_alpha != 0)
        {
          SDL_SetTextureAlphaMod(image->highlight[idx], alpha);
          SDL_RenderCopyF(m_renderer, image->highlight[idx], &inter, &target);
        }

        ++ idx;
      }
    }
  }
#endif
}

void SDL::draw_line (const int xa, const int ya, const int xb, const int yb,
                     unsigned int red, unsigned green, unsigned blue)
{
#ifndef SOSAGE_GUILESS
  SDL_SetRenderDrawColor(m_renderer, Uint8(red), Uint8(green), Uint8(blue), 255);
  SDL_RenderDrawLine (m_renderer, xa, ya, xb, yb);
#endif
}

void SDL::draw_square (const int x, const int y, const int size,
                     unsigned int red, unsigned green, unsigned blue)
{
#ifndef SOSAGE_GUILESS
  SDL_Rect rect;
  rect.x = x - size / 2;
  rect.y = y - size / 2;
  rect.w = size;
  rect.h = size;

  SDL_SetRenderDrawColor(m_renderer, Uint8(red), Uint8(green), Uint8(blue), 255);
  SDL_RenderFillRect(m_renderer, &rect);
#endif
}

void SDL::draw_rectangle (const int x, const int y, const int width, const int height,
                          unsigned int red, unsigned green, unsigned blue, unsigned alpha)
{
#ifndef SOSAGE_GUILESS
  SDL_Rect rect;
  rect.x = x - width / 2;
  rect.y = y - height / 2;
  rect.w = width;
  rect.h = height;

  SDL_SetRenderDrawColor(m_renderer, Uint8(red), Uint8(green), Uint8(blue), Uint8(alpha));
  SDL_RenderFillRect(m_renderer, &rect);
#endif
}

void SDL::end ()
{
#ifndef SOSAGE_GUILESS
  SDL_RenderPresent (m_renderer);
#else
  SDL_Delay(20); // If no GUI, simulate GPU delay
#endif
}

} // namespace Sosage::Third_party
