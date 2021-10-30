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

#include <Sosage/Utils/binary_io.h>

namespace Sosage
{

void binary_write (std::ostream& os, const std::size_t& t)
{
  unsigned int tt = static_cast<unsigned int>(t);
  binary_write (os, tt);
}

void binary_write (std::ostream& os, const Buffer& b)
{
  os.write (b.data(), b.size());
}

void binary_write (std::ostream& os, const std::string& str)
{
  os.write (str.data(), str.size());
}

void binary_read (std::istream& is, Buffer& b)
{
  is.read (b.data(), b.size());
}

} // namespace Sosage
