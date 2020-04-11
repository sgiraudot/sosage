#include <Sosage/Component/Image.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Image::Image (const std::string& id, int w, int h, int r, int g, int b, int a)
  : Base(id), m_origin(0,0), m_z(Sosage::interface_depth), m_on(true),
    m_box_collision (false)
{
  m_core = Core::Graphic::create_rectangle (w, h, r, g, b, a);
}

Image::Image (const std::string& id, const std::string& file_name, int z)
  : Base(id), m_origin(0,0), m_z(z), m_on(true),
    m_box_collision (false)
{
  m_core = Core::Graphic::load_image (file_name);
}

Image::Image (const std::string& id, Font_handle font, const std::string& color_str,
              const std::string& text, bool outlined)
  : Base(id), m_origin(0,0), m_z(Sosage::inventory_back_depth), m_on(true),
    m_box_collision (true)
{
  if (outlined)
    m_core = Core::Graphic::create_outlined_text (font->core(), color_str, text);
  else
    m_core = Core::Graphic::create_text (font->core(), color_str, text);
}

Image::~Image()
{
  Core::Graphic::delete_image(m_core);
}

std::string Image::str() const
{
  return this->id() + " at (" + std::to_string (m_origin.x())
    + ";" + std::to_string(m_origin.y())
    + ";" + std::to_string(m_z) + "), " + (m_on ? "ON" : "OFF");
}

void Image::set_relative_origin (double ratio_x, double ratio_y)
{
  m_origin = Point (width() * ratio_x, height() * ratio_y);
}


void Image::rescale (int z)
{
  m_z = z;
  double scaling = z / double(Sosage::world_depth);
  Core::Graphic::rescale (m_core, scaling);
}

void Image::set_scale (double scale)
{
  Core::Graphic::rescale (m_core, scale);
}

bool Image::is_target_inside (int x, int y) const
{
  return Core::Graphic::is_inside_image (m_core, x, y);
}


} // namespace Sosage::Component
