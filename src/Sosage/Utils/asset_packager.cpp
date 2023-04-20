/*
    [src/Sosage/Utils/asset_packager.cpp]
    Package (and depackage) assets.

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

#ifdef SOSAGE_SCAP

#include <Sosage/Component/Ground_map.h>
#include <Sosage/Third_party/LZ4.h>
#include <Sosage/Third_party/SDL.h>
#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/asset_packager.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/binary_io.h>
#include <Sosage/Utils/image_split.h>

#include <SDL_image.h>

#include <filesystem>
#include <fstream>
#include <sstream>

#include <tbb/parallel_for_each.h>

namespace Sosage::SCAP
{

std::size_t package_size_before = 0;
std::size_t package_size_after = 0;

std::string package (const std::string& filename)
{
  std::string out = "";

  if (contains(filename, "locale.yaml") ||
      std::count(filename.begin(), filename.end(), '.') == 3) // locale files have 3 points, like image.fr_FR.sdl_surface.lz4
    return "locale";

  for (const std::string& p : packages)
  {
    std::string pp = p;
    std::replace (pp.begin(), pp.end(), '_', '/');
    if (contains(filename, pp) && p.size() > out.size())
      out = p;
  }

  return (out == "" ? "general" : out);
}

using Output_file = std::shared_ptr<std::ofstream>;
using Package_files = std::unordered_map<std::string, Output_file>;

Package_files open_packages (const std::string& root)
{
  Package_files out;
  for (const std::string& p : packages)
    out.insert (std::make_pair (p, std::make_shared<std::ofstream>(root + "/" + p + ".data", std::ios::binary)));
  return out;
}

inline void display_compression (std::size_t before, std::size_t after)
{
  if (after > before)
    std::cerr << " -> counterproductive compression (";
  else if (int(std::round(before / double(after)) < 2))
    std::cerr << " -> compressed by " << 100. * (before - after) / double(before) << "% (";
  else
    std::cerr << " -> compressed x" << int(std::round(before / double(after))) << " (";

  std::size_t min = std::min(before, after);

  if (min > 1024 * 1024)
    std::cerr << before / (1024 * 1024) << " MB  ->  " << after / (1024 * 1024) << " MB)" << std::endl;
  else if (min > 1024)
    std::cerr << before / 1024 << " kB  ->  " << after / 1024 << " kB)" << std::endl;
  else
    std::cerr << before << " B  ->  " << after << " B)" << std::endl;
}

void write_file (std::ofstream& ofile, const std::string& filename)
{
  std::ifstream ifile (filename);
  std::ostringstream oss;
  oss << ifile.rdbuf();
  std::string str = oss.str();
  Buffer buffer (str.begin(), str.end());
  std::size_t size_before = buffer.size();
  package_size_before += size_before;

  binary_write (ofile, buffer.size());
  Buffer cbuffer = lz4_compress_buffer (buffer.data(), buffer.size());
  buffer.swap(cbuffer);
  std::size_t size_after = buffer.size();
  package_size_after += size_after;
  display_compression (size_before, size_after);

  binary_write (ofile, buffer.size());
  binary_write (ofile, buffer);
}

void write_image (std::ofstream& ofile, const std::string& filename, bool is_object)
{
  SDL_Surface* input = IMG_Load (filename.c_str());
  Third_party::SDL::fix_transparent_borders(input);
  SDL_PixelFormat* format = SDL_AllocFormat(surface_format);
  SDL_Surface *output = SDL_ConvertSurface(input, format, 0);
  SDL_FreeFormat(format);
  SDL_FreeSurface(input);

  Bitmap_2 mask;
  if (is_object)
    mask = Third_party::SDL::create_mask(output);

  SDL_LockSurface(output);
  unsigned short width = (unsigned short)(output->w);
  unsigned short height = (unsigned short)(output->h);
  unsigned int bpp = (unsigned int)(output->format->BytesPerPixel);
  SDL_UnlockSurface(output);

  binary_write (ofile, width);
  binary_write (ofile, height);
  binary_write (ofile, surface_format);

  std::vector<SDL_Surface*> tiles;
  if (endswith (filename, "_map.png"))
  {
    tiles.push_back(output);
  }
  else
  {
    tiles = Splitter::split_image(output);
    SDL_FreeSurface(output);
    std::cerr << " -> image splitted into " << tiles.size() << std::endl;
  }

  std::size_t parallel_size = (is_object ? 2 * tiles.size() : tiles.size());
  std::vector<std::size_t> index (tiles.size());
  for (std::size_t i = 0; i < index.size(); ++ i)
    index[i] = i;
  std::vector<std::size_t> size_before (parallel_size);
  std::vector<std::size_t> size_after (parallel_size);
  std::vector<Buffer> buffer (parallel_size);

  // 8min15 100% seq
  // 5min00 limit 8
  // 4min17 100% parallel
  // 4min13 limit 3
  auto compress_images = [&](const std::size_t& idx)
  {
    SDL_Surface* tile = tiles[idx];
    std::size_t iidx = (is_object ? idx*2 : idx);
    SDL_LockSurface(tile);
    unsigned int size = bpp * tile->w * tile->h;
    size_before[iidx] = size;

    buffer[iidx] = lz4_compress_buffer (tile->pixels, size);
    SDL_UnlockSurface(tile);
    size_after[iidx] = buffer[iidx].size();

    // Highlight
    if (is_object)
    {
      SDL_Surface* high = SDL_CreateRGBSurfaceWithFormat (0, tile->w, tile->h, 32, surface_format);
      SDL_FillRect(high, nullptr, SDL_MapRGBA(high->format, Uint8(0), Uint8(0), Uint8(0), Uint8(0)));
      SDL_BlitSurface (tile, nullptr, high, nullptr);

      Third_party::SDL::Surface_access access (high);
      for (std::size_t j = 0; j < access.height(); ++ j)
        for (std::size_t i = 0; i < access.width(); ++ i)
          access.set(i,j, {255, 255, 255, (unsigned char)(0.5 * access.get(i,j)[3])});
      access.release();

      SDL_LockSurface(high);
      size_before[iidx+1] = size;
      buffer[iidx+1] = lz4_compress_buffer (high->pixels, size);
      SDL_UnlockSurface(high);
      size_after[iidx+1] = buffer[iidx].size();
      SDL_FreeSurface (high);
    }
    SDL_FreeSurface (tile);
  };

  std::size_t limit = 3;
  if (tiles.size() > limit)
    tbb::parallel_for_each (index.begin(), index.end(), compress_images);
  else
    std::for_each (index.begin(), index.end(), compress_images);

  std::size_t total_size_before = 0;
  std::size_t total_size_after = 0;

  for (std::size_t i = 0; i < buffer.size(); ++ i)
  {
    total_size_before += size_before[i];
    binary_write (ofile, buffer[i].size());
    binary_write (ofile, buffer[i]);
    total_size_after += size_after[i];

  }

  if (is_object)
  {
    std::size_t size_before = mask.size();
    total_size_before += size_before;
    Buffer buffer = lz4_compress_buffer (mask.data(), size_before);
    binary_write (ofile, size_before);
    binary_write (ofile, buffer.size());
    binary_write (ofile, buffer);
    std::size_t size_after = buffer.size();
    total_size_after += size_after;
  }

  package_size_before += total_size_before;
  package_size_after += total_size_after;

  display_compression (total_size_before, total_size_after);
}


void compile_package (const std::string& input_folder, const std::string& output_folder)
{
  std::cerr.precision(3);
  // Prepare package
  std::filesystem::create_directory(output_folder + "/data/");
  std::filesystem::create_directory(output_folder + "/resources");
  std::filesystem::copy(input_folder + "/resources", output_folder + "/resources");
  std::filesystem::copy(input_folder + "/config.cmake", output_folder);
  std::string root = input_folder + "/data/";

  Package_files files = open_packages(output_folder + "/data/");
  Asset_manager::init(root, true);

  std::size_t root_size = root.size();
  if (root[root.size() - 1] != '/')
    root_size ++;

  for(const auto& p: std::filesystem::recursive_directory_iterator(root))
  {
    if (std::filesystem::is_directory(p))
      continue;
    std::string abs_path = p.path();
    std::string path = std::string(abs_path.begin() + root_size, abs_path.end());

    std::string extension = std::string(path.begin() + path.find_last_of('.') + 1, path.end());
    if (extension == "data")
      continue;

    if (contains (path, "_map.png"))
    {
      std::cerr << "Packaging " << path << " precomputed graph" << std::endl;
      std::string abs_map_path = abs_path;
      abs_map_path.resize(abs_map_path.size() - 4);
      abs_map_path += ".graph";


      Component::Ground_map map ("", "", path, 0, 0, []{});
      map.write(abs_map_path);

      std::string map_path = std::string(abs_map_path.begin() + root_size, abs_map_path.end());
      map_path += ".lz4";

      unsigned char path_size = map_path.size();
      Output_file file = files[package(path)];
      binary_write (*file, path_size);
      binary_write (*file, map_path);
      write_file (*file, abs_map_path);
    }

    if (extension == "png")
    {
      path.resize(path.size() - 3);
      path += "sdl_surface.lz4";
    }
    else
      path += ".lz4";

    std::cerr << "Packaging " << path << std::endl;

    if (path.size() >= 255)
      std::cerr << "Warning: path exceeds 255 char limit: " << path << std::endl;

    unsigned char path_size = path.size();
    Output_file file = files[package(path)];
    binary_write (*file, path_size);
    binary_write (*file, path);

    if (extension == "png")
      write_image (*file, abs_path, contains(path, "images/objects") ||
                   contains(path, "images/interface") || contains(path, "images/inventory") || contains(path, "images/masks"));
    else
    {
      if (extension != "yaml" && extension != "ogg" && extension != "ttf" && extension != "txt")
        std::cerr << "Warning: unknown extension " << extension << std::endl;
      write_file (*file, abs_path);
    }
  }
  unsigned char zero_size = 0;
  for (auto& f : files)
    binary_write(*(f.second), zero_size);

  std::cerr << "All done" << std::endl;
  display_compression (package_size_before, package_size_after);
}

void decompile_package (const std::string& ifolder, const std::string& folder)
{
  if (!Asset_manager::init(ifolder))
  {
    debug << "Asset manager could not use " << ifolder << std::endl;
    return;
  }

  for (const auto& asset : Asset_manager::asset_map())
  {
    const std::string& fname = asset.first;
    auto pos = fname.find(".png.");
    if (pos != std::string::npos)
    {
      std::string postfix (fname.begin() + pos + 4, fname.end());
      bool is_hl = false;
      auto pos2 = postfix.find(".HL");
      if (pos2 != std::string::npos)
      {
        is_hl = true;
        postfix.resize(pos2);
      }
      std::string iname = fname;
      iname.resize(pos);
      std::string aname = iname + postfix + ".png";
      iname += ".png";
      if (postfix == ".mask")
        continue;
      debug << aname << ": ";

      pos2 = postfix.find('x');
      Uint32 x = std::atoi(std::string(postfix.begin() + 1, postfix.begin() + pos2).c_str());
      Uint32 y = std::atoi(std::string(postfix.begin() + pos2 + 1, postfix.end()).c_str());
      int width = -1, height = -1;
      int format_int = -1;
      std::tie (width, height, format_int) = Asset_manager::image_info (iname);
      SDL_Rect rect = Splitter::rect (width, height, x, y);
      if (contains(iname, "_map.png"))
      {
        rect.w = width;
        rect.h = height;
      }
      debug << rect.w << "x" << rect.h << std::endl;

      SDL_Surface* surf = SDL_CreateRGBSurfaceWithFormat (0, rect.w, rect.h, 32, format_int);
      SDL_LockSurface (surf);
      Asset_manager::open (iname, surf->pixels, x, y, is_hl);

      std::string ofname = aname;
      std::replace (ofname.begin(), ofname.end(), '/', '_');
      ofname = folder + ofname;
      if (is_hl)
      {
        ofname.resize(ofname.size() - 3);
        ofname += "HL.png";
      }

      SDL_UnlockSurface (surf);
      IMG_SavePNG(surf, ofname.c_str());
      SDL_FreeSurface(surf);
    }
    else if (!contains(fname, ".png"))
    {
      debug << fname << std::endl;
      std::string ofname = fname;
      std::replace (ofname.begin(), ofname.end(), '/', '_');
      ofname = folder + ofname;

      Asset asset = Asset_manager::open(fname);
      std::ofstream ofile(ofname, std::ios::binary);
      binary_write (ofile, *asset.buffer());
      asset.close();
    }
  }
}

} // namespace Sosage::SCAP

#endif
