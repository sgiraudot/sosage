/*
  [src/Sosage/Utils/binary_io.cpp]
  Read and write variables in binary.

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

#ifndef SOSAGE_UTILS_BINARY_IO_H
#define SOSAGE_UTILS_BINARY_IO_H

#include <iostream>
#include <vector>

namespace Sosage
{

using Buffer = std::vector<char>;


template <typename T>
void binary_write (std::ostream& os, const T& t)
{
  os.write (reinterpret_cast<const char*>(&t), sizeof(T));
}

inline void binary_write (std::ostream& os, const std::size_t& t)
{
  unsigned int tt = static_cast<unsigned int>(t);
  binary_write (os, tt);
}

inline void binary_write (std::ostream& os, const Buffer& b)
{
  os.write (b.data(), b.size());
}

inline void binary_write (std::ostream& os, const std::string& str)
{
  os.write (str.data(), str.size());
}

template <typename T>
T binary_read (std::istream& is)
{
  T t;
  is.read (reinterpret_cast<char*>(&t), sizeof(T));
  return t;
}

inline void binary_read (std::istream& is, Buffer& b)
{
  is.read (b.data(), b.size());
}


} // namespace Sosage

#endif // BINARY_IO_H
