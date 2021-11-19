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

#include <SDL.h>

#include <string>

namespace Sosage::Third_party::SDL_file
{

struct Asset
{
  SDL_RWops* buffer = nullptr;
  std::size_t size = 0;
  operator bool() const;
};

Asset open (const std::string& filename, bool write = false);
Asset open (const void* memory, std::size_t size);
std::size_t read (Asset asset, void* ptr, std::size_t max_num);
void write (Asset asset, const char* str);
std::size_t tell (Asset asset);
void seek (Asset asset, std::size_t pos);
void close (Asset asset);
std::string base_path();
std::string pref_path();

} // namespace Sosage::Third_party::SDL_file

#endif // SOSAGE_THIRD_PARTY_SDL_H
