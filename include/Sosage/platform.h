/*
  [include/Sosage/Utils/platform.h]
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

#ifndef SOSAGE_PLATFORM_H
#define SOSAGE_PLATFORM_H

#if defined(__ANDROID__)
#  define SOSAGE_ANDROID
#elif defined(__APPLE__)
#  define SOSAGE_MAC
#elif defined(_WIN32)
#  define SOSAGE_WINDOWS
#elif defined(__linux__)
#  define SOSAGE_LINUX
#endif

#endif // SOSAGE_PLATFORM_H
