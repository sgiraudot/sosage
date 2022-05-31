/*
  [src/Sosage/Utils/error.cpp]
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

#include <Sosage/Utils/error.h>

#ifdef SOSAGE_ANDROID
#include <android/log.h>
#endif

#if defined(SOSAGE_EMSCRIPTEN) || defined(SOSAGE_WINDOWS)
#include <SDL.h>
#endif

namespace Sosage
{

char* dbg_location = nullptr;

#ifdef SOSAGE_DEBUG_BUFFER
int Debug_buffer::sync()
{
#if defined(SOSAGE_ANDROID)
  __android_log_print (ANDROID_LOG_DEBUG, "Sosage", "%s", this->str().c_str());
#else
  SDL_Log("%s", this->str().c_str());
#endif
  this->str("");
  return 0;
}
#endif

void check_impl (const char* file, int line, const std::string& str)
{
#if defined(SOSAGE_DEBUG) && !defined(SOSAGE_ANDROID)
  throw std::runtime_error(std::string(dbg_location) + ": " + str + " [" + file + ":" + std::to_string(line) + "]" );
#else
  debug << "Error in " << dbg_location << ": "<< str << " [" << file << ":" << line << "]" << std::endl;
  exit(EXIT_FAILURE);
#endif
}

} // namespace Sosage
