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

#include <Sosage/Third_party/LZ4.h>
#include <Sosage/Third_party/SDL_file.h>
#include <Sosage/Utils/profiling.h>

#include <unordered_map>

namespace Sosage
{

namespace IO = Third_party::SDL_file;

constexpr auto packages = { "general", "images", "images_cutscenes", "sounds" };

class Asset_manager;

class Asset
{
  IO::Asset m_base;
  Buffer* m_buffer;

  Asset (const std::string& filename, bool write = false)
    : m_buffer(nullptr)
  { m_base = IO::open(filename, write); }
  Asset (Buffer* buffer)
    : m_buffer(buffer)
  { m_base = IO::open(buffer->data(), buffer->size()); }
  Asset (const void* memory, std::size_t size)
  { m_base = IO::open(memory, size); }

  friend Asset_manager;
public:

  Asset() { }
  void close()
  {
    IO::close(m_base);
    if (m_buffer != nullptr)
    {
      delete m_buffer;
      m_buffer = nullptr;
    }
  }
  SDL_RWops* base() { return m_base.buffer; }
  std::size_t size() const { return m_base.size; }
  std::size_t read (void* ptr, std::size_t max_num)
  { return IO::read (m_base, ptr, max_num); }
  void write (const std::string& str)
  { IO::write (m_base, str.c_str()); }
  std::size_t tell() { return IO::tell(m_base); }
  void seek (std::size_t pos) { IO::seek(m_base, pos); }

  template <typename T>
  T binary_read ()
  {
    T t;
    IO::read (m_base, &t, sizeof(T));
    return t;
  }

  void binary_read (Buffer& b)
  {
    IO::read (m_base, b.data(), b.size());
  }

};

struct Packaged_asset
{
  std::size_t buffer_id = 0;
  std::size_t position = 0;
  std::size_t compressed_size = 0;
  std::size_t size = 0;

  // Only used by images
  unsigned short width = 0;
  unsigned short height = 0;
  unsigned int format = 0;

  Buffer buffer;
};

using Package_asset_map = std::unordered_map<std::string, Packaged_asset>;

class Asset_manager
{
  static std::string folder_name;
  static std::vector<Buffer> buffers;
  static Package_asset_map package_asset_map;

public:

  static bool packaged() { return !buffers.empty(); }

  static void init (const std::string& folder, bool scap_mode = false)
  {
    folder_name = folder;
    if (scap_mode)
      return;


    try
    {
      Asset asset = open ("general.data");
      asset.close();
      buffers.resize(packages.size());
    }
    catch (Sosage::No_such_file&)
    {
      Asset asset = open ("data/init.yaml");
      asset.close();
    }

    if (packaged())
    {
      SOSAGE_TIMER_START(Asset_manager__depackage);
      std::size_t buffer_id = 0;
      for (const std::string& package : packages)
      {
        Asset asset = open (package + ".data", true);

        std::size_t end = 0;
        while (true)
        {
          auto path_size = asset.binary_read<unsigned char>();
          if (path_size == 0)
            break;

          Buffer path (path_size);
          asset.binary_read(path);
          std::string fname = std::string (path.begin(), path.end());

          Packaged_asset passet;
          passet.buffer_id = buffer_id;

          auto ext = fname.find(".sdl_surface.lz4");
          if (ext != std::string::npos) // custom surface
          {
            passet.width = asset.binary_read<unsigned short>();
            passet.height = asset.binary_read<unsigned short>();
            passet.format = asset.binary_read<unsigned int>();
            SDL_PixelFormat* pixel_format = SDL_AllocFormat(passet.format);
            unsigned int bpp = (unsigned int)(pixel_format->BytesPerPixel);
            SDL_FreeFormat(pixel_format);
            passet.size = bpp * passet.width * passet.height;
            passet.compressed_size = asset.binary_read<unsigned int>();
            passet.position = asset.tell();
            end = passet.position + passet.compressed_size;
            asset.seek(end);

            fname.resize(ext);
            fname = fname + ".png";
          }
          else if (fname.find(".lz4") == std::string::npos) // uncompressed file
          {
            passet.size = asset.binary_read<unsigned int>();
            passet.position = asset.tell();
            end = passet.position + passet.size;
            asset.seek(end);
          }
          else
          {
            passet.size = asset.binary_read<unsigned int>();
            passet.compressed_size = asset.binary_read<unsigned int>();
            passet.position = asset.tell();
            end = passet.position + passet.compressed_size;
            asset.seek(end);
            fname.resize(fname.size() - 4);
          }
          package_asset_map.insert (std::make_pair (fname, passet));
        }

        asset.seek(0);
        buffers[buffer_id].resize(end);
        asset.read(buffers[buffer_id].data(), end);
        asset.close();

        ++ buffer_id;
      }
#if 0
      for (const auto& p : package_asset_map)
      {
        std::cerr << p.first << " is in buffer " << p.second.buffer_id
                  << " at position " << p.second.position << " with compressed size "
                  << p.second.compressed_size << " and size " << p.second.size << std::endl;
      }
#endif
      SOSAGE_TIMER_STOP(Asset_manager__depackage);

    }
  }

  static Asset open_pref (const std::string& filename, bool write = false)
  {
    return Asset (IO::pref_path() + filename, write);
  }

  static Asset open (const std::string& filename, bool file_is_package = false)
  {
    if (packaged() && !file_is_package)
    {
      auto iter = package_asset_map.find(filename);
      check (iter != package_asset_map.end(), "Packaged asset " + filename + " not found");
      Packaged_asset& asset = iter->second;
      if (asset.compressed_size == 0) // uncompressed file
        return Asset (buffers[asset.buffer_id].data() + asset.position, asset.size);
      // else
      Buffer* buffer = new Buffer(asset.size);
      lz4_decompress_buffer (buffers[asset.buffer_id].data() + asset.position, asset.compressed_size,
          buffer->data(), asset.size);
      return Asset (buffer);
    }
    // else
    return Asset (local_file_name(filename));
  }

  static std::tuple<int, int, int> image_info (const std::string& filename)
  {
    auto iter = package_asset_map.find(filename);
    check (iter != package_asset_map.end(), "Packaged asset " + filename + " not found");
    Packaged_asset& asset = iter->second;
    return std::make_tuple(asset.width, asset.height, asset.format);
  }

  static void open (const std::string& filename, void* memory)
  {
    auto iter = package_asset_map.find(filename);
    check (iter != package_asset_map.end(), "Packaged asset " + filename + " not found");
    Packaged_asset& asset = iter->second;

    lz4_decompress_buffer (buffers[asset.buffer_id].data() + asset.position, asset.compressed_size,
        memory, asset.size);
  }


private:

  static std::string local_file_name (const std::string& filename)
  {
    return folder_name + filename;
  }
};


} // namespace Sosage

#endif // SOSAGE_UTILS_ASSET_MANAGER_H
