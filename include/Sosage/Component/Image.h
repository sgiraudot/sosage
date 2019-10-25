#ifndef SOSAGE_COMPONENT_IMAGE_H
#define SOSAGE_COMPONENT_IMAGE_H

#include <Sosage/Component/Handle.h>
#include <Sosage/third_party_config.h>

namespace Sosage::Component
{

class Image : public Base
{
public:
  typedef Graphic_core Core;

private:
  Core::Image m_core;
  Core::Image m_original;
  int m_z;
  bool m_on;
  
public:

  Image (const std::string& id, const std::string& file_name, int z);
  virtual ~Image();

  const Core::Image& core() const { return m_core; }

  const bool& on() const { return m_on; }
  bool& on() { return m_on; }
  
  virtual int xmin() const { return 0; }
  virtual int xmax() const { return Core::width(m_core); }
  virtual int ymin() const { return 0; }
  virtual int ymax() const { return Core::height(m_core); }

  int width() const { return xmax() - xmin(); }
  int height() const { return ymax() - ymin(); }
  
  const int& z() const { return m_z; }
  int& z() { return m_z; }

  void rescale (int z);
};

typedef std::shared_ptr<Image> Image_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_IMAGE_H
