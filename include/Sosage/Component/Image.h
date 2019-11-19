#ifndef SOSAGE_COMPONENT_IMAGE_H
#define SOSAGE_COMPONENT_IMAGE_H

#include <Sosage/Component/Font.h>
#include <Sosage/Component/Handle.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/geometry.h>

namespace Sosage::Component
{

class Image : public Base
{
private:
  Core::Graphic::Image m_core;
  Point m_origin;
  int m_z;
  bool m_on;
  
public:

  Image (const std::string& id, int w, int h, int r = 0, int g = 0, int b = 0, int a = 255);
  Image (const std::string& id, const std::string& file_name, int z);
  Image (const std::string& id, Font_handle font, const std::string& color_str, const std::string& text);
  
  virtual ~Image();

  virtual std::string str() const
  {
    return this->id() + " at (" + std::to_string (m_origin.x())
      + ";" + std::to_string(m_origin.y())
      + ";" + std::to_string(m_z) + "), " + (m_on ? "ON" : "OFF");
  }

  const Core::Graphic::Image& core() const { return m_core; }

  const Point& origin() const { return m_origin; }
  Point& origin() { return m_origin; }
  
  const bool& on() const { return m_on; }
  bool& on() { return m_on; }
  
  virtual int xmin() const { return 0; }
  virtual int xmax() const { return Core::Graphic::width(m_core); }
  virtual int ymin() const { return 0; }
  virtual int ymax() const { return Core::Graphic::height(m_core); }

  int width() const { return xmax() - xmin(); }
  int height() const { return ymax() - ymin(); }
  
  const int& z() const { return m_z; }
  int& z() { return m_z; }

  void rescale (int z);
  void set_scale (double scale);
};

typedef std::shared_ptr<Image> Image_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_IMAGE_H
