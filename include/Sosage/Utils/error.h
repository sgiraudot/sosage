#ifndef SOSAGE_UTILS_ERROR_H
#define SOSAGE_UTILS_ERROR_H

#include <iostream>

namespace Sosage
{

#if defined(SOSAGE_ERRORS_AS_EXCEPTIONS)
inline void error (const std::string& str)
{
  throw std::runtime_error(str);
}
#else
inline void error (const std::string& str)
{
  std::cerr << "Error: " << str << std::endl;
  exit(EXIT_FAILURE);
}
#endif

#if not defined(NDEBUG)
inline void debug (const std::string& str)
{
  std::cerr << str << std::endl;
}
#else
inline void debug (const std::string&)
{
}
#endif

} // namespace Sosage

#endif // SOSAGE_UTILS_ERROR_H
