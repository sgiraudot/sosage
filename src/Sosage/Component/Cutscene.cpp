/*
  [src/Sosage/Component/Cutscene.cpp]
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

#include <Sosage/Component/Cutscene.h>
#include <Sosage/Utils/error.h>

#include <algorithm>

namespace Sosage::Component
{

Cutscene::Cutscene (const std::string& id)
  : Base(id), m_starting_time(-1), m_paused_time(-1)
{ }

double Cutscene::current_time (const double &time, bool paused)
{
  if (paused)
  {
    if (m_paused_time < 0)
      m_paused_time = time;
    return -1;
  }

  if (m_starting_time < 0)
  {
    m_starting_time = time;
    m_paused_time = -1;
    return 0;
  }

  if (m_paused_time > 0)
  {
    m_starting_time += (time - m_paused_time);
    m_paused_time = -1;
  }

  return time - m_starting_time;
}

std::vector<Cutscene::Element>::iterator Cutscene::begin()
{
  return m_display.begin();
}

std::vector<Cutscene::Element>::iterator Cutscene::end()
{
  return m_display.end();
}

Cutscene::Element& Cutscene::add (const std::string& id)
{
  m_display.push_back ({id, {}, true});
  return m_display.back();
}

void Cutscene::add (const std::string& id, double begin_time,
                    double end_time, int x, int y, int z,
                    double zoom)
{
  Element& el = add(id);
  el.keyframes.push_back ({begin_time, x, y, z, zoom});
  if (end_time > 0)
    el.keyframes.push_back ({end_time, x, y, z, zoom});
}

void Cutscene::finalize()
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

void Cutscene::get_frame (double time, const Element& element,
                          int& x, int& y, int& z, double& zoom)
{
  std::size_t keyframe_id = 1;
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
  if (ratio < 0.) ratio = 0.;
  if (ratio > 1.) ratio = 1.;

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


} // namespace Sosage::Component
