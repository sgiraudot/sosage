/*
  [include/Sosage/Component/Cutscene.h]
  Timeline info for cutscenes.

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

#ifndef SOSAGE_COMPONENT_CUTSCENE_H
#define SOSAGE_COMPONENT_CUTSCENE_H

#include <Sosage/Component/Base.h>

#include <vector>

namespace Sosage::Component
{

class Cutscene : public Base
{
public:
  struct Keyframe
  {
    double time;
    int x, y, z;
    double zoom;
  };

  struct Element
  {
    std::string id;
    std::vector<Keyframe> keyframes;
    bool active;
  };

private:
  std::vector<Element> m_display;
  double m_starting_time;
  double m_paused_time;

public:

  Cutscene (const std::string& id);
  double current_time (const double &time, bool paused);
  std::vector<Element>::iterator begin();
  std::vector<Element>::iterator end();
  Element& add (const std::string& id);
  void add (const std::string& id, double begin_time,
            double end_time = -1, int x = -1, int y = -1, int z = -1,
            double zoom = 1);
  void finalize();
  void get_frame (double time, const Element& element,
                  int& x, int& y, int& z, double& zoom);
};

using Cutscene_handle = std::shared_ptr<Cutscene>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CUTSCENE_H
