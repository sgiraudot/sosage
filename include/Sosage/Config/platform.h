/*
  [include/Sosage/Config/platform.h]
  Internal defines for platform handling.

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

#ifndef SOSAGE_CONFIG_PLATFORM_H
#define SOSAGE_CONFIG_PLATFORM_H

namespace Sosage
{

namespace Config
{

#if defined(__ANDROID__) && !defined(SOSAGE_NATIVE_ANDROID)
#define SOSAGE_ANDROID
constexpr bool android = true;
constexpr bool mac = false;
constexpr bool windows = false;
constexpr bool gnunux = false;
constexpr bool emscripten = false;

#elif defined(__APPLE__)
#define SOSAGE_MAC
constexpr bool android = false;
constexpr bool mac = true;
constexpr bool windows = false;
constexpr bool gnunux = false;
constexpr bool emscripten = false;

#elif defined(_WIN32)
#define SOSAGE_WINDOWS
constexpr bool android = false;
constexpr bool mac = false;
constexpr bool windows = true;
constexpr bool gnunux = false;
constexpr bool emscripten = false;
#define WINVER 0x0600 // Enable use of locale functions

#elif defined(__linux__)
#define SOSAGE_GNUNUX
constexpr bool android = false;
constexpr bool mac = false;
constexpr bool windows = false;
constexpr bool gnunux = true;
constexpr bool emscripten = false;

#elif defined(__EMSCRIPTEN__)
#define SOSAGE_EMSCRIPTEN
constexpr bool android = false;
constexpr bool mac = false;
constexpr bool windows = false;
constexpr bool gnunux = false;
constexpr bool emscripten = true;

#endif

}

}

#endif // SOSAGE_CONFIG_PLATFORM_H
