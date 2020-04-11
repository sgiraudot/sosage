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
  bool m_box_collision;
  
public:

  Image (const std::string& id, int w, int h, int r = 0, int g = 0, int b = 0, int a = 255);
  Image (const std::string& id, const std::string& file_name, int z);
  Image (const std::string& id, Font_handle font, const std::string& color_str,
         const std::string& text, bool outlined = false);
  virtual ~Image();
  virtual std::string str() const;
  void set_relative_origin (double ratio_x, double ratio_y);
  const Core::Graphic::Image& core() const { return m_core; }
  const Point& origin() const { return m_origin; }
  Point& origin() { return m_origin; }
  const bool& on() const { return m_on; }
  bool& on() { return m_on; }
  const bool& box_collision() const { return m_box_collision; }
  bool& box_collision() { return m_box_collision; }
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
  bool is_target_inside (int x, int y) const;
};

typedef std::shared_ptr<Image> Image_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_IMAGE_H
