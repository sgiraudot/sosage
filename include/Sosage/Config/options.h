/*
  [include/Sosage/Config/options.h]
  Compile-time options passed by CMake.

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

#ifndef SOSAGE_CONFIG_OPTIONS_H
#define SOSAGE_CONFIG_OPTIONS_H

#if !defined(NDEBUG) || defined(SOSAGE_CFG_DISPLAY_DEBUG_INFO)
#define SOSAGE_DEBUG
#endif

#ifdef SOSAGE_CFG_PROFILE
#define SOSAGE_PROFILE
#define SOSAGE_PROFILE_FINELY
#define SOSAGE_PROFILE_TO_FILE
#endif

#ifdef SOSAGE_CFG_GUILESS
#define SOSAGE_GUILESS
#endif

#ifdef SOSAGE_CFG_USE_SDL_TIME
#define SOSAGE_SDL_TIME
#endif

#ifndef SOSAGE_PREF_PATH
#define SOSAGE_PREF_PATH "ptilouk"
#endif

#ifndef SOSAGE_PREF_SUBPATH
#define SOSAGE_PREF_SUBPATH "sosage"
#endif

#endif // SOSAGE_CONFIG_OPTIONS_H
