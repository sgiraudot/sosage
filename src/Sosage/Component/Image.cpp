/*
  [src/Sosage/Component/Image.cpp]
  Basis for all graphic items.

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

#include <Sosage/Component/Image.h>
#include <Sosage/Config/config.h>

namespace Sosage::Component
{

Image::Image (const std::string& id, int w, int h, int r, int g, int b, int a)
  : Base(id), m_origin(0,0), m_z(Config::interface_depth), m_on(true),
    m_collision(BOX)
{
  m_core = Core::Graphic::create_rectangle (w, h, r, g, b, a);
}

Image::Image (const std::string& id, const std::string& file_name, int z,
              const Collision_type& collision)
  : Base(id), m_origin(0,0), m_z(z), m_on(true),
    m_collision (collision)
{
  m_core = Core::Graphic::load_image (file_name, (collision == PIXEL_PERFECT),
                                      (collision != UNCLICKABLE) && z != 0);
}

Image::Image (const std::string& id, Font_handle font, const std::string& color_str,
              const std::string& text, bool outlined)
  : Base(id), m_origin(0,0), m_z(Config::inventory_depth), m_on(true),
    m_collision (BOX)
{
  if (outlined)
    m_core = Core::Graphic::create_outlined_text (font->core(), color_str, text);
  else
    m_core = Core::Graphic::create_text (font->core(), color_str, text);
}

// Image that share the same Graphic core
Image::Image (const std::string& id, std::shared_ptr<Image> copy)
  : Base(id), m_core (copy->m_core), m_origin(copy->m_origin),
    m_z(copy->m_z), m_on(copy->m_on), m_collision(copy->m_collision)
{

}

std::string Image::str() const
{
  return this->id() + " at (" + std::to_string (m_origin.x())
    + ";" + std::to_string(m_origin.y())
    + ";" + std::to_string(m_z) + "), " + (m_on ? "ON" : "OFF");
}

void Image::set_collision (const Collision_type& collision)
{
  if (m_collision == PIXEL_PERFECT && collision != PIXEL_PERFECT)
    m_core.free_mask();
  m_collision = collision;
}

void Image::set_relative_origin (double ratio_x, double ratio_y)
{
  m_origin = Point (width() * ratio_x, height() * ratio_y);
}


void Image::rescale (double z)
{
  m_z = static_cast<int>(z);
  double scaling = z / double(Config::world_depth);
  Core::Graphic::rescale (m_core, scaling);
}

void Image::set_scale (double scale)
{
  Core::Graphic::rescale (m_core, scale);
}

bool Image::is_target_inside (int x, int y) const
{
  check (m_core.mask != nullptr, "Checkin pixel perfect collision without mask");
  dbg_check (x < width() && y < height(),
             "Out of bound pixel " + std::to_string(x) + "/" + std::to_string(width())
             + " x " + std::to_string(y) + "/" + std::to_string(height()));
  return (*m_core.mask)(x, y);
}


} // namespace Sosage::Component
