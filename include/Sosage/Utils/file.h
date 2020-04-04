#ifndef SOSAGE_UTILS_FILE_H
#define SOSAGE_UTILS_FILE_H

#include <Sosage/Third_party/SDL_file.h>

namespace Sosage
{

typedef Third_party::SDL_file::File File;

inline File open (const std::string& filename)
{
  return Third_party::SDL_file::open (filename);
}

inline std::size_t read (File file, void* ptr, std::size_t max_num)
{
  return Third_party::SDL_file::read (file, ptr, max_num);
}

inline void close (File file)
{
  return Third_party::SDL_file::close (file);
}

} // namespace Sosage

#endif // SOSAGE_UTILS_FILE_H
