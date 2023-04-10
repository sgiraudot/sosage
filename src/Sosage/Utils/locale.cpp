/*
  [src/Sosage/Utils/locale.cpp]
  Accessing locale info on all platforms.

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

#include <Sosage/Config/platform.h>
#include <Sosage/Utils/locale.h>

#include <SDL_locale.h>


namespace Sosage
{

std::vector<std::string> get_locales()
{
  SDL_Locale* locale = SDL_GetPreferredLocales();
  std::vector<std::string> out;
  while (locale->language != nullptr)
  {
    std::string language = locale->language;
    if (locale->country != nullptr)
      language += "_" + std::string(locale->country);
    out.push_back (language);
    ++ locale;
  }
  return out;
}

} // namespace Sosage
