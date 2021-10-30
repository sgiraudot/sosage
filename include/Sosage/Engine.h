/*
  [include/Sosage/Engine.h]
  Inits all systems, holds content and runs main loop.

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

#ifndef SOSAGE_ENGINE_H
#define SOSAGE_ENGINE_H

#include <Sosage/Content.h>
#include <Sosage/System/Base.h>
#include <Sosage/Utils/error.h>

namespace Sosage
{

class Engine
{
  Content m_content;
  std::vector<System::Handle> m_systems;

public:

  Engine (int argc, char** argv);
  ~Engine();
  int run (const std::string& folder_name);
  bool run();

private:

  void handle_cmdline_args (int argc, char** argv);
};

} // namespace Sosage

#endif // SOSAGE_ENGINE_H
