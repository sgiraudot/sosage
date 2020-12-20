/*
  [include/Sosage/Component/Image.h]
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

#ifndef SOSAGE_COMPONENT_IMAGE_H
#define SOSAGE_COMPONENT_IMAGE_H

#include <Sosage/Component/Font.h>
#include <Sosage/Component/Handle.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/enum.h>
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
  Collision_type m_collision;

public:

  Image (const std::string& id, int w, int h, int r = 0, int g = 0, int b = 0, int a = 255);
  Image (const std::string& id, const std::string& file_name, int z = 0,
         const Collision_type& collision = UNCLICKABLE);
  Image (const std::string& id, Font_handle font, const std::string& color_str,
         const std::string& text, bool outlined = false);
  virtual ~Image() { /*debug("Destructor of ", id());*/ }
  virtual std::string str() const;
  void set_relative_origin (double ratio_x, double ratio_y);
  const Core::Graphic::Image& core() const { return m_core; }
  const Point& origin() const { return m_origin; }
  Point& origin() { return m_origin; }
  const bool& on() const { return m_on; }
  bool& on() { return m_on; }
  const Collision_type& collision() const { return m_collision; }
  void set_collision(const Collision_type& collision);
  virtual int xmin() const { return 0; }
  virtual int xmax() const { return Core::Graphic::width(m_core); }
  virtual int ymin() const { return 0; }
  virtual int ymax() const { return Core::Graphic::height(m_core); }
  int width() const { return xmax() - xmin(); }
  int height() const { return ymax() - ymin(); }
  const int& z() const { return m_z; }
  int& z() { return m_z; }
  void rescale (double z);
  void set_scale (double scale);
  void set_alpha (unsigned char alpha) { m_core.alpha = alpha; }
  bool is_target_inside (int x, int y) const;
};

using Image_handle = std::shared_ptr<Image>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_IMAGE_H
