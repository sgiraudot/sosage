/*
  [include/Sosage/Third_party/LZ4.h]
  Wrapper for LZ4 compression library.

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

#ifndef SOSAGE_THIRD_PARTY_LZ4_H
#define SOSAGE_THIRD_PARTY_LZ4_H

#include <Sosage/Utils/binary_io.h>

namespace Sosage
{

Buffer lz4_compress_buffer (void* data, std::size_t size);
void lz4_decompress_buffer (void* data, std::size_t size, void* out, std::size_t output_size);

}

#endif // SOSAGE_THIRD_PARTY_LZ4_H
