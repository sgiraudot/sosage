/*
  [include/Sosage/Component/Cropped.h]
  Specialization of Image for cropped objects.

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

#ifndef SOSAGE_COMPONENT_CROPPED_H
#define SOSAGE_COMPONENT_CROPPED_H

#include <Sosage/Component/Image.h>

namespace Sosage::Component
{

class Cropped : public Image
{
public:

  int m_xmin;
  int m_xmax;
  int m_ymin;
  int m_ymax;

public:

  Cropped (const std::string& entity, const std::string& component, const std::string& file_name, int z);
  void crop (int xmin, int xmax, int ymin, int ymax);
  virtual int xmin() const;
  virtual int xmax() const;
  virtual int ymin() const;
  virtual int ymax() const;

  STR_NAME("Cropped");
};

using Cropped_handle = std::shared_ptr<Cropped>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CROPPED_H
