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

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/geometry.h>

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

    Keyframe (double time, int x, int y, int z, double zoom)
      : time (time), x(x), y(y), z(z), zoom(zoom) { }
  };

  struct Element
  {
    std::string id;
    std::vector<Keyframe> keyframes;
    bool active;

    Element (const std::string& id) : id(id), active(true) { }
  };

private:
  std::vector<Element> m_display;
  double m_starting_time;
  double m_paused_time;

public:

  Cutscene (const std::string& id) : Base(id), m_starting_time(-1), m_paused_time(-1) { }

  double current_time(const double &time, bool paused)
  {
    if (paused)
    {
      std::cerr << "Pause " << time << std::endl;
      if (m_paused_time < 0)
        m_paused_time = time;
      return -1;
    }

    if (m_paused_time > 0)
    {
      std::cerr << "Unpause " << time - m_paused_time << std::endl;
      m_starting_time += (time - m_paused_time);
      m_paused_time = -1;
    }

    if (m_starting_time < 0)
    {
      m_starting_time = time;
      return 0;
    }
    return time - m_starting_time;
  }

  std::vector<Element>::iterator begin() { return m_display.begin(); }
  std::vector<Element>::iterator end() { return m_display.end(); }

  Element& add (const std::string& id)
  {
    m_display.emplace_back (id);
    return m_display.back();
  }
  void add (const std::string& id, double begin_time,
            double end_time = -1, int x = -1, int y = -1, int z = -1,
            double zoom = 1)
  {
    Element& el = add(id);
    el.keyframes.emplace_back (begin_time, x, y, z, zoom);
    if (end_time > 0)
      el.keyframes.emplace_back (end_time, x, y, z, zoom);
  }

  void finalize()
  {
    for (const Element& el : m_display)
    {
      check (!el.keyframes.empty(), "Element " + el.id + " doesn't have keyframes");
    }

    std::sort (m_display.begin(), m_display.end(),
               [](const Element& a, const Element& b) -> bool
               {
                 return a.keyframes.front().time < b.keyframes.front().time;
               });

#ifdef SOSAGE_DEBUG
    std::cerr << "Cutscene " << this->id() << std::endl;
    for (const Element& el : m_display)
    {
      std::cerr << " * " << el.id << ":" << std::endl;
      std::size_t i = 0;
      for (const Keyframe k : el.keyframes)
        std::cerr << "   - " << ++ i << ": " << k.time << "s ["
                  << k.x << " " << k.y << " " << k.z << "] "
                  << k.zoom << std::endl;
    }
#endif
  }

  void get_frame (double time, const Element& element,
                  int& x, int& y, int& z, double& zoom)
  {
    std::size_t keyframe_id = 0;
    while (keyframe_id != element.keyframes.size()
           && element.keyframes[keyframe_id].time < time)
      ++ keyframe_id;

    if (keyframe_id == element.keyframes.size())
    {
      x = element.keyframes.back().x;
      y = element.keyframes.back().y;
      z = element.keyframes.back().z;
      zoom = element.keyframes.back().zoom;
      return;
    }

    double ratio = (time - element.keyframes[keyframe_id - 1].time)
                   / (element.keyframes[keyframe_id].time - element.keyframes[keyframe_id - 1].time);

    // Squat zoom for ratio if fadein/fadeout
    if (element.id == "fadein" || element.id == "fadeout")
    {
      zoom = ratio;
      return;
    }

    int xbefore = element.keyframes[keyframe_id-1].x;
    int xafter = element.keyframes[keyframe_id].x;
    int ybefore = element.keyframes[keyframe_id-1].y;
    int yafter = element.keyframes[keyframe_id].y;
    int zbefore = element.keyframes[keyframe_id-1].z;
    int zafter = element.keyframes[keyframe_id].z;
    double zoombefore = element.keyframes[keyframe_id-1].zoom;
    double zoomafter = element.keyframes[keyframe_id].zoom;

    if (xbefore == xafter)
      x = xbefore;
    else
      x = ratio * xafter + (1. - ratio) * xbefore;

    if (ybefore == yafter)
      y = ybefore;
    else
      y = ratio * yafter + (1. - ratio) * ybefore;

    if (zbefore == zafter)
      z = zbefore;
    else
      z = ratio * zafter + (1. - ratio) * zbefore;

    if (zoombefore == zoomafter)
      zoom = zoombefore;
    else
      zoom = ratio * zoomafter + (1. - ratio) * zoombefore;
  }


};

using Cutscene_handle = std::shared_ptr<Cutscene>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_CUTSCENE_H
