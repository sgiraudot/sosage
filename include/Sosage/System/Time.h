/*
  [include/Sosage/System/Time.h]
  Time handling (framerate, sleep, etc.).

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

#ifndef SOSAGE_SYSTEM_TIME_H
#define SOSAGE_SYSTEM_TIME_H

#include <Sosage/Content.h>
#include <Sosage/System/Base.h>
#include <Sosage/Utils/Clock.h>

namespace Sosage
{

namespace Config
{
constexpr double speedup_factor = 2.;
}

namespace System
{

class Time : public Base
{
  Clock m_clock;
  bool m_loading;

public:

  Time (Content& content);

  virtual void run();
  void signal_loading() { m_loading = true; }

private:

  void limit_fps();
};

} // namespace System
} // namespace Sosage


#endif // SOSAGE_SYSTEM_TIME_H
