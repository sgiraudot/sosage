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

#include <Sosage/Config/config.h>
#include <Sosage/Utils/Resource_manager.h>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include <array>

namespace Sosage::Third_party
{

class SDL
{
public:

  using Surface_manager = Resource_manager<SDL_Surface>;
  using Texture_manager = Resource_manager<SDL_Texture>;
  using Font_manager = Resource_manager<TTF_Font>;

  using Surface = typename Surface_manager::Resource_handle;
  using Texture = typename Texture_manager::Resource_handle;
  using Font_base = typename Font_manager::Resource_handle;

  struct Image
  {
    Surface surface;
    Texture texture;
    double scaling;
    unsigned char alpha;

    Image (Surface surface = Surface(), Texture texture = Texture(), double scaling = 1., unsigned char alpha = 255)
      : surface (surface), texture (texture), scaling (scaling), alpha(alpha)
    { }
  };

  using Font = std::pair<Font_base, Font_base>;

  static SDL_Window* m_window;
  static SDL_Renderer* m_renderer;
  static Surface_manager m_surfaces;
  static Texture_manager m_textures;
  static Font_manager m_fonts;

public:

  static Image create_rectangle (int w, int h, int r, int g, int b, int a);
  static Surface load_surface (const std::string& file_name);
  static Image load_image (const std::string& file_name);
  static Font load_font (const std::string& file_name, int size);
  static SDL_Color black();
  static SDL_Color color (const std::string& color_str);
  static Image create_text (const Font& font, const std::string& color_str,
                            const std::string& text);
  static Image create_outlined_text (const Font& font, const std::string& color_str,
                                     const std::string& text);
  static void rescale (Image& source, double scaling);
  static std::array<unsigned char, 3> get_color (Surface image, int x, int y);
  static bool is_inside_image (Image image, int x, int y);
  static int width (Surface image);
  static int height (Surface image);
  static int width (Image image);
  static int height (Image image);

  SDL ();
  ~SDL ();

  void init (int& window_width, int& window_height, bool fullscreen);

  void update_window (const std::string& name, const Image& icon);
  void update_view(int interface_width, int interface_height);
  void toggle_fullscreen(bool fullscreen);
  void get_window_size (int& w, int& h);
  void begin(int interface_width, int interface_height);
  void create_texture (const Image& image);
  void draw (const Image& image,
             const int xsource, const int ysource,
             const int wsource, const int hsource,
             const int xtarget, const int ytarget,
             const int wtarget, const int htarget);
  void draw_line (const int xa, const int ya, const int xb, const int yb,
                  unsigned int red = 255, unsigned green = 0, unsigned blue = 0);
  void draw_square (const int x, const int b, const int size,
                    unsigned int red = 255, unsigned green = 0, unsigned blue = 0);
  void end();

};

} // namespace Sosage::Third_party

#endif // SOSAGE_THIRD_PARTY_SDL_H
