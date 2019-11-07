#include <Sosage/Component/Image.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Image::Image (const std::string& id, const std::string& file_name, int z)
  : Base(id), m_z(z), m_on(true)
{
  std::cerr << "Creating image " << file_name << std::endl;
  m_core = Core::Graphic::load_image (file_name);
}

Image::~Image()
{
  Core::Graphic::delete_image(m_core);
}

void Image::rescale (int z)
{
  double scaling = z / double(config().world_depth);
  Core::Graphic::rescale (m_core, scaling);
}


} // namespace Sosage::Component
