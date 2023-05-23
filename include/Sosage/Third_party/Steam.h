/*
  [include/Sosage/Third_party/Steam.h]
  Wrapper for Steam SDK.

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

#ifndef SOSAGE_THIRD_PARTY_STEAM_H
#define SOSAGE_THIRD_PARTY_STEAM_H

#include <string>

namespace Sosage::Steam
{

extern bool achievement_init;

void init();
void run();
std::string game_language();
bool set_achievement (const std::string& id);
void shutdown();

}

#endif // SOSAGE_THIRD_PARTY_STEAM_H
