/*
  [src/Sosage/Third_party/SDL_file.cpp]
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

#include <Sosage/Config/options.h>
#include <Sosage/Third_party/SDL_file.h>
#include <Sosage/Utils/error.h>

namespace Sosage::Third_party::SDL_file
{

Asset::operator bool() const
{
  return (buffer != nullptr);
}

Asset open (const std::string& filename, bool write)
{
  Asset out;
  out.buffer = SDL_RWFromFile(filename.c_str(), write ? "w" : "r");
  if (out.buffer != nullptr)
    out.size = std::size_t(SDL_RWsize (out.buffer));
  return out;
}

Asset open (const void* memory, std::size_t size)
{
  Asset out;
  out.buffer = SDL_RWFromConstMem(memory, int(size));
  if (out.buffer != nullptr)
    out.size = std::size_t(SDL_RWsize (out.buffer));
  return out;
}

std::size_t read (Asset asset, void* ptr, std::size_t max_num)
{
  return std::size_t(SDL_RWread(asset.buffer, ptr, 1, max_num));
}

void write (Asset asset, const char* str)
{
  SDL_RWwrite (asset.buffer, str, 1, SDL_strlen(str));
}

std::size_t tell (Asset asset)
{
  return std::size_t(SDL_RWtell(asset.buffer));
}

void seek (Asset asset, std::size_t pos)
{
  SDL_RWseek (asset.buffer, Sint64(pos), RW_SEEK_SET);
}

void close (Asset asset)
{
  SDL_RWclose (asset.buffer);
}

std::string base_path()
{
  char* bp = SDL_GetBasePath();
  std::string out = bp;
  free(bp);
  return out;
}

std::string pref_path()
{
  char* pp = SDL_GetPrefPath(SOSAGE_PREF_PATH, SOSAGE_PREF_SUBPATH);
  std::string out = pp;
  free(pp);
  return out;
}

} // namespace Sosage::Third_party::SDL_file
