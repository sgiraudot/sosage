#include <Sosage/Component/Image.h>

namespace Sosage::Component
{

Image::Image (const std::string& file_name, int z)
  : m_z(z)
{
  std::cerr << "Creating image " << file_name << std::endl;
  m_core = Core::load_image (file_name);
}

} // namespace Sosage::Component
