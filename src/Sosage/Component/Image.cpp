#include <Sosage/Component/Image.h>
#include <Sosage/Config.h>

namespace Sosage::Component
{

Image::Image (const std::string& id, const std::string& file_name, int z)
  : Base(id), m_core(nullptr), m_original(nullptr), m_z(z), m_on(true)
{
  std::cerr << "Creating image " << file_name << std::endl;
  m_core = Core::load_image (file_name);
}

Image::~Image()
{
  Core::delete_image(m_core);
}

void Image::rescale (int z)
{
  if (m_original == nullptr)
    m_original = Core::copy_image(m_core);

  m_z = z;
  double scaling = z / double(config().world_depth);

  Core::delete_image(m_core);
  m_core = Core::rescale (m_original, scaling);
}


} // namespace Sosage::Component
