/*
  [include/Sosage/Utils/file.h]
  Low-level platform-independent file IO functions.

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

#ifndef SOSAGE_UTILS_FILE_H
#define SOSAGE_UTILS_FILE_H

#include <Sosage/Third_party/SDL_file.h>

namespace Sosage
{

typedef Third_party::SDL_file::File File;

inline File open (const std::string& filename)
{
  return Third_party::SDL_file::open (filename);
}

inline std::size_t read (File file, void* ptr, std::size_t max_num)
{
  return Third_party::SDL_file::read (file, ptr, max_num);
}

inline void close (File file)
{
  return Third_party::SDL_file::close (file);
}

} // namespace Sosage

#endif // SOSAGE_UTILS_FILE_H
