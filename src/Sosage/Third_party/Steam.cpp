/*
  [src/Sosage/Third_party/Steam.cpp]
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

#include <Sosage/Third_party/Steam.h>
#include <Sosage/Utils/error.h>

#ifdef SOSAGE_LINKED_WITH_STEAMSDK
#include <steam/steam_api.h>
#endif

namespace Sosage::Steam
{

bool achievement_init = false;

void init()
{
#ifdef SOSAGE_LINKED_WITH_STEAMSDK
  if (SteamAPI_RestartAppIfNecessary (SOSAGE_STEAM_APP_ID)) // Remplacez par votre AppID
    exit(1);
  bool success = SteamAPI_Init();
  check(success, "Couldn't init Steam SDK.");

  if (SteamUser()->BLoggedOn())
  {
    SteamUserStats()->RequestCurrentStats();
    achievement_init = true;
  }
#endif
}

void run()
{
#ifdef SOSAGE_LINKED_WITH_STEAMSDK
  SteamAPI_RunCallbacks();
#endif
}

std::string game_language()
{
#ifdef SOSAGE_LINKED_WITH_STEAMSDK
  return SteamApps()->GetCurrentGameLanguage();
#else
  return "";
#endif
}

void set_achievement (const std::string& id)
{
#ifdef SOSAGE_LINKED_WITH_STEAMSDK
  if (achievement_init)
  {
    SteamUserStats()->SetAchievement(id.c_str());
    SteamUserStats()->StoreStats();
  }
#endif
}

void shutdown()
{
#ifdef SOSAGE_LINKED_WITH_STEAMSDK
  SteamAPI_Shutdown();
#endif
}

} // namespace Sosage
