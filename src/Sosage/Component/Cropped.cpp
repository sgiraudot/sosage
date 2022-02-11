/*
  [src/Sosage/Component/Cropped.cpp]
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

#include <Sosage/Component/Cropped.h>
#include <Sosage/Core/Graphic.h>

namespace Sosage::Component
{

Cropped::Cropped (const std::string& entity, const std::string& component, const std::string& file_name, int z)
  : Image(entity, component, file_name, z)
  , m_xmin (0)
  , m_xmax (Core::Graphic::width(core()))
  , m_ymin (0)
  , m_ymax (Core::Graphic::height(core()))
{
}

void Cropped::crop (int xmin, int xmax, int ymin, int ymax)
{
  m_xmin = xmin;
  m_xmax = xmax;
  m_ymin = ymin;
  m_ymax = ymax;
}

int Cropped::xmin() const
{
  return m_xmin;
}

int Cropped::xmax() const
{
  return m_xmax;
}

int Cropped::ymin() const
{
  return m_ymin;
}

int Cropped::ymax() const
{
  return m_ymax;
}

} // namespace Sosage::Component
