#ifndef SOSAGE_THIRD_PARTY_SDL_FILE_H
#define SOSAGE_THIRD_PARTY_SDL_FILE_H

#include <Sosage/Config.h>
#include <Sosage/platform.h>

#ifdef SOSAGE_ANDROID
#  include <SDL.h>
#else
#  include <SDL2/SDL.h>
#endif

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
  check (out.buffer != nullptr, "Cannot open " + filename);
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
