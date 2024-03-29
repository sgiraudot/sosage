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

#include <Sosage/Component/Base.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/enum.h>
#include <Sosage/Utils/geometry.h>

namespace Sosage
{

namespace Config
{
constexpr int world_depth = 3240;
} // namespace Config

namespace Component
{

class Image : public Base
{
private:
  Core::Graphic::Image m_core;
  Point m_origin;
  int m_z;
  bool m_on;
  Collision_type m_collision;
  double m_scaling;
  unsigned char m_highlight_alpha;
  unsigned char m_alpha;

public:

  Image (const std::string& entity, const std::string& component,
         int w, int h, int r = 0, int g = 0, int b = 0, int a = 255);
  Image (const std::string& entity, const std::string& component, const std::string& file_name, int z = 0,
         const Collision_type& collision = UNCLICKABLE, bool with_highlight = false);
  Image (const std::string& entity, const std::string& component,
         Font_handle font, const std::string& color_str,
         const std::string& text, bool outlined = false);
  Image (const std::string& entity, const std::string& component, std::shared_ptr<Image> copy);
  void compose_with (const std::shared_ptr<Image>& other);
  void set_relative_origin (double ratio_x, double ratio_y);
  const Core::Graphic::Image& core() const;
  const Point& origin() const;
  Point& origin();
  const bool& on() const;
  bool& on();
  const Collision_type& collision() const;
  void set_collision(const Collision_type& collision);
  virtual int xmin() const;
  virtual int xmax() const;
  virtual int ymin() const;
  virtual int ymax() const;
  int width() const;
  int height() const;
  const int& z() const;
  int& z();
  double scale() const;
  void rescale (double z);
  void set_scale (double scale);
  void set_alpha (unsigned char alpha);
  unsigned char alpha() const;
  void set_highlight (unsigned char alpha);
  unsigned char highlight() const;
  bool is_target_inside (int x, int y) const;

  STR_NAME("Image");
  STR_VALUE(std::string(m_on ? "ON" : "OFF") + ", z=" + to_string(m_z)
            + ", scale=" + to_string(m_scaling)
            + ", alpha=" + to_string(m_alpha)
            + ", highlight=" + to_string(m_highlight_alpha));
};

using Image_handle = std::shared_ptr<Image>;

} // namespace Component

} // namespace Sosage

#endif // SOSAGE_COMPONENT_IMAGE_H
