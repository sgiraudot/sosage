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
#include <Sosage/Config/options.h>
#include <Sosage/Config/platform.h>
#include <Sosage/Utils/error.h>

#include <SDL.h>

#include <cstdlib>

namespace Sosage::Third_party::SDL_file
{

struct File
{
  SDL_RWops* buffer;
  std::size_t size;
};

inline File open (const std::string& filename, bool write = false)
{
  File out;
  out.buffer = SDL_RWFromFile(filename.c_str(), write ? "w" : "r");
  if (out.buffer == nullptr)
  {
    debug ("File ", filename, " not found");
    throw Sosage::No_such_file();
  }

  out.size = std::size_t(SDL_RWsize (out.buffer));
  return out;
}

inline std::size_t read (File file, void* ptr, std::size_t max_num)
{
  return std::size_t(SDL_RWread(file.buffer, ptr, 1, max_num));
}

inline void write (File file, const char* str)
{
  SDL_RWwrite (file.buffer, str, 1, SDL_strlen(str));
}

inline void close (File file)
{
  SDL_RWclose (file.buffer);
}

inline std::string base_path()
{
  char* bp = SDL_GetBasePath();
  std::string out = bp;
  free(bp);
  return out;
}

inline std::string pref_path()
{
  char* pp = SDL_GetPrefPath(SOSAGE_PREF_PATH, SOSAGE_PREF_SUBPATH);
  std::string out = pp;
  free(pp);
  return out;
}

} // namespace Sosage::Third_party::SDL_file

#endif // SOSAGE_THIRD_PARTY_SDL_H
