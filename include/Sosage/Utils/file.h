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

using File = Third_party::SDL_file::File;

inline File open (const std::string& filename, bool write = false)
{
  return Third_party::SDL_file::open (filename, write);
}

inline std::size_t read (File file, void* ptr, std::size_t max_num)
{
  return Third_party::SDL_file::read (file, ptr, max_num);
}

inline void write (File file, const std::string& str)
{
  Third_party::SDL_file::write (file, str.c_str());
}

inline void close (File file)
{
  return Third_party::SDL_file::close (file);
}

inline std::string base_path()
{
  return Third_party::SDL_file::base_path();
}

inline std::string pref_path()
{
  return Third_party::SDL_file::pref_path();
}

} // namespace Sosage

#endif // SOSAGE_UTILS_FILE_H
