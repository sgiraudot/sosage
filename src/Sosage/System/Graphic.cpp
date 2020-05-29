/*
  [src/Sosage/System/Graphic.cpp]
  Renders the content of the game at each frame.

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

#include <Sosage/Component/Event.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Graphic.h>

#include <algorithm>
#include <vector>

namespace Sosage::System
{

Graphic::Graphic (Content& content)
  : m_content (content)
{ }

void Graphic::init()
{
  auto iw = m_content.get<Component::Int>("window:width");
  auto ih = m_content.get<Component::Int>("window:height");
  int w = iw->value();
  int h = ih->value();
  m_core.init (m_content.get<Component::String>("game:name")->value(),
               w, h,
               m_content.get<Component::Boolean>("window:fullscreen")->value());
  iw->set(w);
  ih->set(h);
}

void Graphic::run()
{
  if (m_content.request<Component::Event>("window:rescaled"))
  {
    m_core.update_view (m_content.get<Component::Int>("interface:width")->value(),
                        m_content.get<Component::Int>("interface:height")->value());
    m_content.remove ("window:rescaled");
  }
  if (m_content.request<Component::Event>("window:toggle_fullscreen"))
  {
    m_core.toggle_fullscreen (m_content.get<Component::Boolean>("window:fullscreen")->value());
    m_content.remove ("window:toggle_fullscreen");
  }

  
  std::vector<Component::Image_handle> images;

  m_core.begin();

  get_images (images);
  display_images (images);
  images.clear();

  m_core.end();
}

void Graphic::get_images (std::vector<Component::Image_handle>& images)
{
  for (const auto& e : m_content)
    if (auto img = Component::cast<Component::Image>(e))
      images.push_back(img);
}

void Graphic::display_images (std::vector<Component::Image_handle>& images)
{
  std::sort (images.begin(), images.end(),
             [](Component::Image_handle a, Component::Image_handle b) -> bool
             {
               return (a->z() < b->z());
             });

  bool locked = (m_content.get<Component::String>("game:status")->value() == "locked");
  
  for (const auto& img : images)
  {
    if (img->on())
    {
      if (locked &&
           img->entity() == "cursor")
        continue;
      
      auto p = m_content.get<Component::Position>(img->entity() + ":position");

      int xmin = img->xmin();
      int ymin = img->ymin();
      int xmax = img->xmax();
      int ymax = img->ymax();
      
      Point screen_position = p->value() - img->core().scaling * Vector(img->origin());

      m_core.draw (img->core(), xmin, ymin, (xmax - xmin), (ymax - ymin),
                   screen_position.x(), screen_position.y(),
                   img->core().scaling * (xmax - xmin),
                   img->core().scaling * (ymax - ymin));
    }
  }

  if (m_content.get<Component::Boolean>("game:debug")->value())
  {
    auto ground_map = m_content.get<Component::Ground_map>("background:ground_map");

    ground_map->for_each_vertex
      ([&](const Point& point)
       {
         m_core.draw_square (point.x(), point.y(), 10);
       });
      
    ground_map->for_each_edge
      ([&](const Point& source, const Point& target)
       {
         m_core.draw_line (source.x(), source.y(),
                           target.x(), target.y());
       });

    auto path = m_content.request<Component::Path>("character:path");
    if (path)
    {
      Point current = m_content.get<Component::Position>("character_body:position")->value();
      m_core.draw_square (current.x(), current.y(), 10, 0, 255, 0);

      for (std::size_t p = path->current(); p < path->size(); ++ p)
      {
        Point next = (*path)[p];
        m_core.draw_square (next.x(), next.y(), 10, 0, 255, 0);
        m_core.draw_line (current.x(), current.y(),
                          next.x(), next.y(), 0, 255, 0);
        current = next;
      }

    }
  }
}

} // namespace Sosage::System
