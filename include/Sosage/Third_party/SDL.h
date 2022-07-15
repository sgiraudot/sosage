/*
  [include/Sosage/Third_party/SDL.h]
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

#ifndef SOSAGE_THIRD_PARTY_SDL_H
#define SOSAGE_THIRD_PARTY_SDL_H

#include <Sosage/Utils/binary_io.h>
#include <Sosage/Utils/Bitmap_2.h>
#include <Sosage/Utils/color.h>
#include <Sosage/Utils/Resource_manager.h>

#include <SDL.h>
#include <SDL_ttf.h>

#include <array>

namespace Sosage
{

namespace Config
{
constexpr int text_outline = 10;
} // namespace Config

namespace Third_party
{

class SDL
{
public:

#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  static constexpr Uint32 rmask = 0xff000000;
  static constexpr Uint32 gmask = 0x00ff0000;
  static constexpr Uint32 bmask = 0x0000ff00;
  static constexpr Uint32 amask = 0x000000ff;
#else
  static constexpr Uint32 rmask = 0x000000ff;
  static constexpr Uint32 gmask = 0x0000ff00;
  static constexpr Uint32 bmask = 0x00ff0000;
  static constexpr Uint32 amask = 0xff000000;
#endif

  using Font_base = std::tuple<TTF_Font*, TTF_Font*, Buffer*>;

  struct Image_base
  {
    SDL_Texture* texture;
    SDL_Texture* highlight;
    Bitmap_2 mask;
    double texture_downscale;
    int width;
    int height;

    Image_base (SDL_Texture* texture = nullptr, SDL_Texture* highlight = nullptr,
                int width = -1, int height = -1);
  };

  static Image_base* make_image (SDL_Texture* texture = nullptr, SDL_Texture* highlight = nullptr,
                                 int width = -1, int height = -1);

  using Image_manager = Resource_manager<Image_base>;
  using Font_manager = Resource_manager<Font_base>;

  using Surface = std::shared_ptr<SDL_Surface>;
  using Image = typename Image_manager::Resource_handle;
  using Font = typename Font_manager::Resource_handle;

  struct Surface_access
  {
    SDL_Surface* surface;
    int bpp;

    Surface_access (SDL_Surface* surface);
    std::size_t width();
    std::size_t height();
    RGBA_color get (std::size_t x, std::size_t y) const;
    void set (std::size_t x, std::size_t y, const RGBA_color& color) const;
    void release();
  };

  static SDL_Window* m_window;
  static SDL_Renderer* m_renderer;
  static SDL_RendererInfo m_info;
  static Image_manager m_images;
  static Font_manager m_fonts;
  Surface m_icon;

public:

  static Image create_rectangle (int w, int h, int r, int g, int b, int a);
  static Image load_image (const std::string& file_name, bool with_mask, bool with_highlight);
  static Image compose (const std::initializer_list<Image>& images);
  static Font load_font (const std::string& file_name, int size);
  static Bitmap_2 create_mask (SDL_Surface* surf);
  static SDL_Color black();
  static SDL_Color color (const std::string& color_str);
  static Image create_text (const Font& font, const std::string& color_str,
                            const std::string& text);
  static Image create_outlined_text (const Font& font, const std::string& color_str,
                                     const std::string& text);
  static bool is_inside_image (Image image, int x, int y);
  static int width (Image image);
  static int height (Image image);

  // Specifically for ground map
  static Surface load_surface (const std::string& file_name);
  static std::array<unsigned char, 3> get_color (Surface image, int x, int y);

  static void display_error(const std::string& error);

  SDL ();
  ~SDL ();

  void clear_managers();

  void init (int& window_width, int& window_height, bool fullscreen);

  void update_window (const std::string& name, const std::string& icon_filename);
  void update_view();
  void toggle_fullscreen(bool fullscreen);
  void toggle_cursor(bool visible);
  void get_window_size (int& w, int& h);
  void begin();
  void draw (const Image& image, unsigned char alpha,
             unsigned char highlight_alpha,
             const int xsource, const int ysource,
             const int wsource, const int hsource,
             const double xtarget, const double ytarget,
             const double wtarget, const double htarget);
  void draw_line (const int xa, const int ya, const int xb, const int yb,
                  unsigned int red = 255, unsigned green = 0, unsigned blue = 0);
  void draw_square (const int x, const int b, const int size,
                    unsigned int red = 255, unsigned green = 0, unsigned blue = 0);
  void draw_rectangle (const int x, const int y, const int width, const int height,
                       unsigned int red, unsigned green, unsigned blue, unsigned alpha);
  void end();

};

} // namespace Third_party

} // namespace Sosage

#endif // SOSAGE_THIRD_PARTY_SDL_H
