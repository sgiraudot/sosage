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

  Bitmap_2 (const std::size_t& width, const std::size_t& height, const bool& value = true);
  bool empty() const;
  void clear();
  void resize (const std::size_t& width, const std::size_t& height, const bool& value = true);
  std::size_t width() const;
  std::size_t height() const;
  std::size_t size() const;
  bool operator() (const std::size_t& x, const std::size_t& y) const;
  reference operator() (const std::size_t& x, const std::size_t& y);
  void swap (Bitmap_2& other);
};

}

#endif // SOSAGE_UTILS_BITMAP_2_H
