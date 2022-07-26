/*
  [src/Sosage/Third_party/LZ4.cpp]
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

#include <Sosage/Third_party/LZ4.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/error.h>

#include <lz4.h>
#include <lz4hc.h>

namespace Sosage
{

Buffer lz4_compress_buffer (void* data, std::size_t size)
{
  const char* cdata = reinterpret_cast<const char*>(data);
  unsigned int max_lz4_size = LZ4_compressBound(size);
  Buffer out (max_lz4_size);
  unsigned int true_size = LZ4_compress_HC(cdata, out.data(), size, max_lz4_size,
                                           LZ4HC_CLEVEL_MAX);
  out.resize(true_size);
  return out;
}

void lz4_decompress_buffer (void* data, std::size_t size, void* out, std::size_t output_size)
{
  const char* cdata = reinterpret_cast<const char*>(data);
  char* cout = reinterpret_cast<char*>(out);
  int decompressed_size = LZ4_decompress_safe (cdata, cout, size, output_size);

  check (std::size_t(decompressed_size) == output_size,
         "LZ4 decompressed size differs from expected ("
         + to_string(decompressed_size) + " != " + to_string(output_size) + ")");
}

} // namespace Sosage
