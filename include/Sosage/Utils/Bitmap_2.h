/*
  [include/Sosage/Utils/Bitmap_2.h]
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

#ifndef SOSAGE_UTILS_BITMAP_2_H
#define SOSAGE_UTILS_BITMAP_2_H


#include <vector>

namespace Sosage
{

class Bitmap_2
{
public:

  using Base = std::vector<bool>;
  using const_reference = typename Base::reference;
  using reference = typename Base::reference;

private:
  Base m_data;
  std::size_t m_width;

public:

  Bitmap_2 (const std::size_t& width, const std::size_t& height, const bool& value = true)
    : m_data (width * height, value), m_width (width)
  {

  }

  bool empty() const { return m_data.empty(); }
  void clear() { m_data.clear(); m_width = 0; }
  void resize (const std::size_t& width, const std::size_t& height, const bool& value = true)
  {
    m_data.resize (width * height, value);
    m_width = width;
  }
  std::size_t width() const { return m_width; }
  std::size_t height() const { return m_data.size() / m_width; }
  std::size_t size() const { return m_data.size(); }

  bool operator() (const std::size_t& x, const std::size_t& y) const
  {
    return m_data[x + m_width * y];
  }
  reference operator() (const std::size_t& x, const std::size_t& y)
  {
    return m_data[x + m_width * y];
  }

  void swap (Bitmap_2& other)
  {
    m_data.swap (other.m_data);
    m_width = other.m_width;
    other.m_width = 0;
  }
};

}

#endif // SOSAGE_UTILS_BITMAP_2_H
