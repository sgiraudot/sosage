/*
  [include/Sosage/Utils/error.h]
  Debug checks, errors, warnings, assertions, exceptions, etc.

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

#ifndef SOSAGE_UTILS_ERROR_H
#define SOSAGE_UTILS_ERROR_H

#include <Sosage/Config/options.h>
#include <Sosage/Config/platform.h>

#ifdef SOSAGE_ANDROID
#include <android/log.h>
#endif

#ifdef SOSAGE_EMSCRIPTEN
#include <SDL.h>
#endif

#include <iostream>

namespace Sosage
{

#define check(test, msg) if (!(test)) check_impl (__FILE__, __LINE__, msg)

#ifdef SOSAGE_DEBUG
#define dbg_check(test, msg) if (!(test)) check_impl (__FILE__, __LINE__, msg)
#else
#define dbg_check(test, msg) (static_cast<void>(0))
#endif

#if defined(SOSAGE_ANDROID)
inline void check_impl (const char* file, int line, const std::string& str)
{
  __android_log_print (ANDROID_LOG_ERROR, "Sosage Error", "%s [%s:%i]", str.c_str(), file, line);
  exit(EXIT_FAILURE);
}
#elif defined(SOSAGE_EMSCRIPTEN)
inline void check_impl (const char* file, int line, const std::string& str)
{
  SDL_Log("%s [%s:%i]", str.c_str(), file, line);
  exit(EXIT_FAILURE);
}
#elif defined(SOSAGE_ASSERTIONS_AS_EXCEPTIONS)
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

#if defined(SOSAGE_DEBUG)
#  if defined(SOSAGE_ANDROID)
inline void debug (const std::string& str)
{
  __android_log_print (ANDROID_LOG_INFO, "Sosage Info", "%s", str.c_str());
}
#  elif defined(SOSAGE_EMSCRIPTEN)
inline void debug (const std::string& str)
{
  SDL_Log("%s", str.c_str());
}
#  else
inline void debug (const std::string& str)
{
  std::cerr << str << std::endl;
}
#  endif
#else
inline void debug (const std::string&)
{
}
#endif

struct No_such_file : public std::exception { };

} // namespace Sosage

#endif // SOSAGE_UTILS_ERROR_H
