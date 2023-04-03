/*
  [include/Sosage/Component/Debug.h]
  Debug information on screen (FPS, etc.).

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

#ifndef SOSAGE_COMPONENT_DEBUG_H
#define SOSAGE_COMPONENT_DEBUG_H

#include <Sosage/Component/Condition.h>
#include <Sosage/Content.h>
#include <Sosage/Utils/Clock.h>

namespace Sosage::Component
{

class Debug : public Boolean
{
private:

  Content& m_content;
  const Clock& m_clock;
  double m_start;
  double m_mean;
  int m_nb;
  double m_cpu;
  double m_mean_cpu;
  int m_mean_nb;
  
public:

  Debug (const std::string& entity, const std::string& component, Content& content, const Clock& clock);
  virtual ~Debug();
  std::string debug_str();

  void start_loop();
  void end_loop();

  STR_NAME("Debug");

private:

  std::string in_game_time();
};

using Debug_handle = std::shared_ptr<Debug>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_DEBUG_H
