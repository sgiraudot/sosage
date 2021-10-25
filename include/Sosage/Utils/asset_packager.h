/*
    [include/Sosage/Utils/asset_packager.h]
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

#ifndef SOSAGE_UTILS_ASSET_PACKAGER_H
#define SOSAGE_UTILS_ASSET_PACKAGER_H

#include <Sosage/Component/Ground_map.h>
#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/binary_io.h>
#include <Sosage/Third_party/SDL.h>

#include <filesystem>
#include <fstream>

namespace Sosage
{

namespace SCAP
{

constexpr unsigned int surface_format = SDL_PIXELFORMAT_ABGR8888;


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

void write_file (std::ofstream& ofile, const std::string& filename, bool compressed)
{
  std::ifstream ifile (filename);
  std::ostringstream oss;
  oss << ifile.rdbuf();
  std::string str = oss.str();
  Buffer buffer (str.begin(), str.end());
  if (compressed)
  {
    std::size_t size_before = buffer.size();
    binary_write (ofile, buffer.size());
    Buffer cbuffer = lz4_compress_buffer (buffer.data(), buffer.size());
    buffer.swap(cbuffer);
    std::size_t size_after = buffer.size();
    std::cerr << " -> compressed by " << 100. * (size_before - size_after) / double(size_before)
              << "%" << std::endl;
    if (size_after > size_before)
      std::cerr << "   WARNING: compression is counterproductive here!" << std::endl;
  }
  binary_write (ofile, buffer.size());
  binary_write (ofile, buffer);
}

void write_image (std::ofstream& ofile, const std::string& filename)
{
  SDL_Surface* input = IMG_Load (filename.c_str());
  SDL_PixelFormat* format = SDL_AllocFormat(surface_format);
  SDL_Surface *output = SDL_ConvertSurface(input, format, 0);
  SDL_FreeFormat(format);
  SDL_FreeSurface(input);

  SDL_LockSurface(output);
  unsigned short width = (unsigned short)(output->w);
  unsigned short height = (unsigned short)(output->h);
  unsigned int bpp = (unsigned int)(output->format->BytesPerPixel);
  unsigned int size = bpp * width * height;

  binary_write (ofile, width);
  binary_write (ofile, height);
  binary_write (ofile, surface_format);

  std::size_t size_before = size;
  Buffer buffer = lz4_compress_buffer (output->pixels, size);
  binary_write (ofile, buffer.size());
  binary_write (ofile, buffer);
  SDL_UnlockSurface(output);
  SDL_FreeSurface (output);
  std::size_t size_after = buffer.size();
  std::cerr << " -> compressed by " << 100. * (size_before - size_after) / double(size_before)
            << "%" << std::endl;
  if (size_after > size_before)
    std::cerr << "   WARNING: compression is counterproductive here!" << std::endl;
}


void compile_package (const std::string& input_folder, const std::string& output_folder)
{
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


      Component::Ground_map map ("", path, 0, 0, []{});
      map.write(abs_map_path);

      std::string map_path = std::string(abs_map_path.begin() + root_size, abs_map_path.end());
      map_path += ".lz4";

      unsigned char path_size = map_path.size();
      Output_file file = files[package(path)];
      binary_write (*file, path_size);
      binary_write (*file, map_path);
      write_file (*file, abs_map_path, true);
    }

    if (extension == "png")
    {
      path.resize(path.size() - 3);
      path += "sdl_surface.lz4";
    }
    else if (extension != "ogg")
      path += ".lz4";

    std::cerr << "Packaging " << path << std::endl;

    if (path.size() >= 255)
      std::cerr << "Warning: path exceeds 255 char limit: " << path << std::endl;

    unsigned char path_size = path.size();
    Output_file file = files[package(path)];
    binary_write (*file, path_size);
    binary_write (*file, path);

    if (extension == "png")
      write_image (*file, abs_path);
    else
    {
      if (extension != "yaml" && extension != "ogg" && extension != "ttf")
        std::cerr << "Warning: unknown extension " << extension << std::endl;
      write_file (*file, abs_path, extension != "ogg");
    }
  }
  unsigned char zero_size = 0;
  for (auto& f : files)
    binary_write(*(f.second), zero_size);
}

void decompile_package (const std::string& filename, const std::string& folder)
{
  std::ifstream ifile (filename, std::ios::binary);

  std::unordered_map<std::string, std::pair<std::size_t, std::size_t> > map;
  std::unordered_map<std::string, std::size_t>  sizes;

  std::size_t end = 0;
  while (true)
  {
    auto path_size = binary_read<unsigned char>(ifile);
    if (path_size == 0)
      break;

    Buffer path (path_size);
    binary_read(ifile, path);
    std::string fname = std::string (path.begin(), path.end());
    std::cerr << fname << std::endl;
    if (contains (fname, ".sdl_surface.lz4"))
    {
      auto width = binary_read<unsigned short>(ifile);
      auto height= binary_read<unsigned short>(ifile);
      auto format_enum = binary_read<unsigned int>(ifile);
      SDL_PixelFormat* format = SDL_AllocFormat(format_enum);
      unsigned int bpp = (unsigned int)(format->BytesPerPixel);
      auto uncompressed_size = bpp * width * height;
      auto file_size = binary_read<unsigned int>(ifile);
      std::size_t begin = ifile.tellg();
      end = begin + file_size;
      ifile.seekg(end);
      sizes.insert (std::make_pair (fname, uncompressed_size));
      map.insert (std::make_pair (fname, std::make_pair(begin, end)));
    }
    else if (!contains (fname, ".lz4")) // uncompressed file
    {
      auto file_size = binary_read<unsigned int>(ifile);
      std::size_t begin = ifile.tellg();
      end = begin + file_size;
      ifile.seekg(end);
      std::string fname = std::string (path.begin(), path.end());
      map.insert (std::make_pair (fname, std::make_pair(begin, end)));
    }
    else
    {
      auto uncompressed_size = binary_read<unsigned int>(ifile);
      auto file_size = binary_read<unsigned int>(ifile);
      std::size_t begin = ifile.tellg();
      end = begin + file_size;
      ifile.seekg(end);
      sizes.insert (std::make_pair (fname, uncompressed_size));
      map.insert (std::make_pair (fname, std::make_pair(begin, end)));
    }
  }

  Buffer buffer(end);
  ifile.seekg(0);
  ifile.read(buffer.data(), end);
  ifile.close();

  for (const auto& m : map)
  {
    std::string p = m.first;
    std::cerr << "Decompressing " << p << std::endl;
    continue;

    std::size_t begin = m.second.first;
    std::size_t end = m.second.second;
    std::size_t output_size = sizes[p];
    std::replace (p.begin(), p.end(), '/', '_');
    if (!contains(p, ".lz4")) // uncompressed file
    {
      std::ofstream ofile(p, std::ios::binary);
      ofile.write (buffer.data() + begin, end - begin);
    }
    else
    {
      p.resize(p.size() - 4);
      p = folder + "/" + p;

      std::ofstream ofile(p, std::ios::binary);
      Buffer out (output_size);
      lz4_decompress_buffer (buffer.data() + begin, (end - begin), out.data(), output_size);
      binary_write (ofile, out);
    }
  }
}

} // namespace SCAP

} // namespace Sosage

#endif // SOSAGE_UTILS_ASSET_PACKAGER_H
