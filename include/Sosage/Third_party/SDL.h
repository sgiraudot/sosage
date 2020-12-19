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
#include <Sosage/Utils/Bitmap_2.h>
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

  using Texture_manager = Resource_manager<SDL_Texture>;
  using Bitmap_manager = Resource_manager<Bitmap_2>;
  using Font_manager = Resource_manager<TTF_Font>;

  using Surface = std::shared_ptr<SDL_Surface>;
  using Texture = typename Texture_manager::Resource_handle;
  using Bitmap = typename Bitmap_manager::Resource_handle;
  using Font_base = typename Font_manager::Resource_handle;

  struct Image
  {
    Texture texture;
    Bitmap mask;
    double scaling;
    double texture_downscale;
    unsigned char alpha;
    int width;
    int height;

    Image (Texture texture = Texture(), Bitmap mask = Bitmap(), int width = -1, int height = -1,
           double scaling = 1., unsigned char alpha = 255)
      : texture (texture), mask(mask),
        scaling (scaling), texture_downscale(1), alpha(alpha),
        width(width), height(height)
    { }

    void free_mask()
    {
      mask = nullptr;
    }
  };

  using Font = std::pair<Font_base, Font_base>;

  static SDL_Window* m_window;
  static SDL_Renderer* m_renderer;
  static SDL_RendererInfo m_info;
  static Texture_manager m_textures;
  static Bitmap_manager m_masks;
  static Font_manager m_fonts;
  SDL_Surface* m_icon;

public:

  static Image create_rectangle (int w, int h, int r, int g, int b, int a);
  static Image load_image (const std::string& file_name, bool with_mask);
  static Font load_font (const std::string& file_name, int size);
  static Bitmap_2* create_mask (SDL_Surface* surf);
  static SDL_Color black();
  static SDL_Color color (const std::string& color_str);
  static Image create_text (const Font& font, const std::string& color_str,
                            const std::string& text);
  static Image create_outlined_text (const Font& font, const std::string& color_str,
                                     const std::string& text);
  static void rescale (Image& source, double scaling);
  static bool is_inside_image (Image image, int x, int y);
  static int width (Image image);
  static int height (Image image);

  // Specifically for ground map
  static Surface load_surface (const std::string& file_name);
  static std::array<unsigned char, 3> get_color (Surface image, int x, int y);

  SDL ();
  ~SDL ();

  void init (int& window_width, int& window_height, bool fullscreen);

  void update_window (const std::string& name, const std::string& icon_filename);
  void update_view(int interface_width, int interface_height);
  void toggle_fullscreen(bool fullscreen);
  void get_window_size (int& w, int& h);
  void begin(int interface_width, int interface_height);
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
