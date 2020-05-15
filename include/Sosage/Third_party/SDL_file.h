/*
  [include/Sosage/Third_party/SDL_file.h]
  Wrapper for SDL library (file system).

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

#ifndef SOSAGE_THIRD_PARTY_SDL_FILE_H
#define SOSAGE_THIRD_PARTY_SDL_FILE_H

#include <Sosage/Config/config.h>
#include <Sosage/Config/platform.h>
#include <Sosage/Utils/error.h>

#include <SDL.h>

namespace Sosage::Third_party::SDL_file
{

struct File
{
  SDL_RWops* buffer;
  std::size_t size;
};

inline File open (const std::string& filename)
{
  File out;
  out.buffer = SDL_RWFromFile(filename.c_str(), "r");
  if constexpr (Config::android)
  {
    check (out.buffer != nullptr, "Cannot read " + filename);
  }
#ifndef SOSAGE_ANDROID // Stupid Android does not get if constexpr
  else
  {
    if (out.buffer == nullptr)
      throw Sosage::Invalid_data_folder();
  }
#endif
  
  out.size = std::size_t(SDL_RWsize (out.buffer));
  return out;
}

inline std::size_t read (File file, void* ptr, std::size_t max_num)
{
  return std::size_t(SDL_RWread(file.buffer, ptr, 1, max_num));
}

inline void close (File file)
{
  SDL_RWclose (file.buffer);
}


} // namespace Sosage::Third_party::SDL_file

#endif // SOSAGE_THIRD_PARTY_SDL_H
