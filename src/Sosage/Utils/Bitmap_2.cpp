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

Bitmap_2::Bitmap_2 (const std::size_t& width, const std::size_t& height, const bool& value)
  : m_data (width * height, value), m_width (width)
{ }

bool Bitmap_2::empty() const
{
  return m_data.empty();
}

void Bitmap_2::clear()
{
  m_data.clear(); m_width = 0;
}

void Bitmap_2::resize (const std::size_t& width, const std::size_t& height, const bool& value)
{
  m_data.resize (width * height, value);
  m_width = width;
}

std::size_t Bitmap_2::width() const
{
  return m_width;
}

std::size_t Bitmap_2::height() const
{
  return m_data.size() / m_width;
}

std::size_t Bitmap_2::size() const
{
  return m_data.size();
}

bool Bitmap_2::operator() (const std::size_t& x, const std::size_t& y) const
{
  return m_data[x + m_width * y];
}

Bitmap_2::reference Bitmap_2::operator() (const std::size_t& x, const std::size_t& y)
{
  return m_data[x + m_width * y];
}

void Bitmap_2::swap (Bitmap_2& other)
{
  m_data.swap (other.m_data);
  m_width = other.m_width;
  other.m_width = 0;
}

} // namespace Sosage
