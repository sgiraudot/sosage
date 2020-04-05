#ifndef SOSAGE_UTILS_COLOR_H
#define SOSAGE_UTILS_COLOR_H

#include <array>
#include <sstream>
#include <string>

namespace Sosage
{

inline std::array<unsigned char, 3> color_from_string (const std::string& str)
{
  std::stringstream ss(str);
  int num;
  ss >> std::hex >> num;
  return { (unsigned char)(num / 0x10000),
           (unsigned char)((num / 0x100) % 0x100),
           (unsigned char)(num % 0x100) };
}


} // namespace Sosage

#endif // SOSAGE_UTILS_COLOR_H
