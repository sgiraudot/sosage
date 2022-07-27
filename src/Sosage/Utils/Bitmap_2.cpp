/*
  [src/Sosage/Utils/Bitmap_2.cpp]
  Bitmap image mask.

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

#include <Sosage/Utils/Bitmap_2.h>

namespace Sosage
{

Bitmap_2::Bitmap_2() : m_width(0) { }

Bitmap_2::Bitmap_2 (const std::size_t& width, const std::size_t& height, const bool& value)
  : m_data (1 + (width * height) / 8, (value ? 255 : 0)), m_width (width)
{ }

bool Bitmap_2::empty() const
{
  return m_data.empty();
}

std::size_t Bitmap_2::width() const
{
  return m_width;
}

std::size_t Bitmap_2::height() const
{
  return (m_data.size() * 8) / m_width;
}

std::size_t Bitmap_2::size() const
{
  return (m_data.size() * 8);
}

bool Bitmap_2::operator() (const std::size_t& x, const std::size_t& y) const
{
  std::size_t idx = x + m_width * y;
  return bool((m_data[idx / 8] >> (idx % 8)) & 1);
}

void Bitmap_2::set (const std::size_t& x, const std::size_t& y, bool value)
{
  std::size_t idx = x + m_width * y;
  if (value)
    m_data[idx / 8] |= (1 << (idx % 8));
  else
    m_data[idx / 8] &= ~(1 << (idx % 8));
}

} // namespace Sosage
