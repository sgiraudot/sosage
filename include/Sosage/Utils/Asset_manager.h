/*
    [include/Sosage/Utils/asset_manager.h]
    Handle compressed or not compressed assets.

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

#ifndef SOSAGE_UTILS_ASSET_MANAGER_H
#define SOSAGE_UTILS_ASSET_MANAGER_H

#include <Sosage/Third_party/SDL_file.h>

namespace Sosage
{

namespace IO = Third_party::SDL_file;

constexpr auto packages = { "general", "images", "images/cutscenes", "sounds" };

class Asset_manager;

class Asset
{
  IO::Asset m_base;

  Asset (const std::string& filename, bool write = false)
  { m_base = IO::open(filename, write); }

  friend Asset_manager;
public:

  Asset() { }
  void close() { IO::close(m_base); }
  SDL_RWops* base() { return m_base.buffer; }
  std::size_t size() const { return m_base.size; }
  std::size_t read (void* ptr, std::size_t max_num)
  { return IO::read (m_base, ptr, max_num); }
  void write (const std::string& str)
  { IO::write (m_base, str.c_str()); }
};

class Asset_manager
{
  static std::string folder_name;

public:

  static bool packaged;

  static void init (const std::string& folder)
  {
    folder_name = folder;
    packaged = false;
    try
    {
      open ("data/init.yaml");
    }
    catch (Sosage::No_such_file&)
    {
      packaged = true;
      open ("general.data");
    }
  }

  static Asset open_pref (const std::string& filename, bool write = false)
  {
    std::cerr << "Open " << IO::pref_path() + filename << std::endl;
    return Asset (IO::pref_path() + filename, write);
  }

  static Asset open (const std::string& filename)
  {
    if (packaged)
      ;
    // else
    return Asset (local_file_name(filename));
  }

private:

  static std::string local_file_name (const std::string& filename)
  {
    return folder_name + filename;
  }
};


} // namespace Sosage

#endif // SOSAGE_UTILS_ASSET_MANAGER_H
