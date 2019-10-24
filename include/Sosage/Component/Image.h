#ifndef SOSAGE_COMPONENT_IMAGE_H
#define SOSAGE_COMPONENT_IMAGE_H

#include <Sosage/Component/Handle.h>
#include <Sosage/third_party_config.h>

namespace Sosage::Component
{

class Image : public Base
{
  typedef Graphic_core Core;

  Core::Image m_core;
  int m_z;
  
public:

  Image (const std::string& file_name, int z);

  const Core::Image& core() const { return m_core; }

  int z() const { return m_z; }
};

typedef std::shared_ptr<Image> Image_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_IMAGE_H
