#ifndef SOSAGE_UTILS_ERROR_H
#define SOSAGE_UTILS_ERROR_H

#include <iostream>

namespace Sosage
{

#define check(test, msg) if (!(test)) check_impl (__FILE__, __LINE__, msg)

#if defined(NDEBUG)
#define dbg_check(test, msg) (static_cast<void>(0))
#else
#define dbg_check(test, msg) if (!(test)) check_impl (__FILE__, __LINE__, msg)
#endif

#define SOSAGE_ASSERTIONS_AS_EXCEPTIONS
#if defined(SOSAGE_ASSERTIONS_AS_EXCEPTIONS)
inline void check_impl (const char* file, int line, const std::string& str)
{
  throw std::runtime_error(str + " [" + file + ":" + std::to_string(line) + "]");
}
#else
inline void check_impl (const char* file, int line, const std::string& str)
{
  std::cerr << "Error: "<< str << " [" << file << ":" << line << "]" << std::endl;
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

inline void output (const std::string& str)
{
  std::cerr << str << std::endl;
}

} // namespace Sosage

#endif // SOSAGE_UTILS_ERROR_H
