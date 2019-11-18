#include <Sosage/Component/Image.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Image::Image (const std::string& id, int w, int h, int r, int g, int b, int a)
  : Base(id), m_origin(0,0), m_z(1000000), m_on(true)
{
  m_core = Core::Graphic::create_rectangle (w, h, r, g, b, a);
}

Image::Image (const std::string& id, const std::string& file_name, int z)
  : Base(id), m_origin(0,0), m_z(z), m_on(true)
{
  debug ("Creating image " + file_name);
  m_core = Core::Graphic::load_image (file_name);
}

Image::Image (const std::string& id, Font_handle font, const std::string& color_str,
              const std::string& text)
  : Base(id), m_origin(0,0), m_z(1000001), m_on(true)
{
  m_core = Core::Graphic::create_text (font->core(), color_str, text);
}

Image::~Image()
{
  Core::Graphic::delete_image(m_core);
}

void Image::rescale (int z)
{
  m_z = z;
  double scaling = z / double(config().world_depth);
  Core::Graphic::rescale (m_core, scaling);
}

void Image::set_scale (double scale)
{
  Core::Graphic::rescale (m_core, scale);
}


} // namespace Sosage::Component
