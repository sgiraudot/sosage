/*
  [include/Sosage/Utils/error.h]
  Debug checks, errors, warnings, assertions, etc.

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

#include <utility>
#include <iostream>
#include <sstream>

namespace Sosage
{

#if !defined(SOSAGE_DEBUG)
#define debug if(false) std::cerr
#elif defined(SOSAGE_ANDROID) || defined(SOSAGE_WINDOWS) || defined(SOSAGE_EMSCRIPTEN)
#define SOSAGE_DEBUG_BUFFER

class Debug_buffer : public std::stringbuf
{
public:
  virtual int sync();
};

extern Debug_buffer debug_buffer;
extern std::ostream debug;
#else
#define debug std::cerr
#endif

#define check(test, msg) if (!(test)) check_impl (__FILE__, __LINE__, msg)

#ifdef SOSAGE_DEBUG
#define dbg_check(test, msg) if (!(test)) check_impl (__FILE__, __LINE__, msg)
#else
#define dbg_check(test, msg) (static_cast<void>(0))
#endif

void check_impl (const char* file, int line, const std::string& str);

} // namespace Sosage

#endif // SOSAGE_UTILS_ERROR_H
