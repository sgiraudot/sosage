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
#include <Sosage/Component/Status.h>
#include <Sosage/Config/config.h>
#include <Sosage/System/Graphic.h>

#include <algorithm>
#include <vector>

namespace Sosage::System
{

namespace C = Component;

Graphic::Graphic (Content& content)
  : Base (content)
{ }

void Graphic::init()
{
  auto iw = get<C::Int>("window:width");
  auto ih = get<C::Int>("window:height");
  int w = iw->value();
  int h = ih->value();
  m_core.init (w, h,
               get<C::Boolean>("window:fullscreen")->value());
  iw->set(w);
  ih->set(h);
}

void Graphic::run()
{
#ifdef SOSAGE_THREADS_ENABLED
  if (get<C::Status>(GAME__STATUS)->value() == LOADING)
  {
    m_core.begin();
    display_spin_loading();
    m_core.end();
    return;
  }
#endif

  if (auto name = request<C::String>("game:name"))
  {
    m_core.update_window (name->value(), get<C::Image>("icon:image")->core());
    remove ("game:name");
  }

  if (request<C::Event>("window:rescaled"))
  {
    m_core.update_view (get<C::Int>("interface:width")->value(),
                        get<C::Int>("interface:height")->value());
    remove ("window:rescaled");
  }
  if (request<C::Event>("window:toggle_fullscreen"))
  {
    m_core.toggle_fullscreen (get<C::Boolean>("window:fullscreen")->value());
    remove ("window:toggle_fullscreen");
  }


  std::vector<C::Image_handle> images;

  m_core.begin();

  get_images (images);
  display_images (images);
  images.clear();

  m_core.end();
}

void Graphic::get_images (std::vector<C::Image_handle>& images)
{
  for (const auto& e : m_content)
    if (auto img = C::cast<C::Image>(e))
      images.push_back(img);
}

void Graphic::display_images (std::vector<C::Image_handle>& images)
{
  std::sort (images.begin(), images.end(),
             [](C::Image_handle a, C::Image_handle b) -> bool
             {
               return (a->z() < b->z());
             });

  auto status = get<C::Status>(GAME__STATUS);
  double xcamera = get<C::Double>(CAMERA__POSITION)->value();

  for (const auto& img : images)
  {
    if (img->on())
    {
      if (status->value() == LOCKED &&
           img->entity() == "cursor")
        continue;
      if (status->value() == LOADING &&
          img->entity() != "loading")
        continue;

      auto position = get<C::Position>(img->entity() + ":position");
      Point p = position->value();

      if (!position->absolute())
        p = p + Vector (-xcamera, 0);

      int xmin = img->xmin();
      int ymin = img->ymin();
      int xmax = img->xmax();
      int ymax = img->ymax();

      Point screen_position = p - img->core().scaling * Vector(img->origin());

      m_core.draw (img->core(), xmin, ymin, (xmax - xmin), (ymax - ymin),
                   screen_position.X(), screen_position.Y(),
                   img->core().scaling * (xmax - xmin),
                   img->core().scaling * (ymax - ymin));
    }
  }

  if (get<C::Boolean>(GAME__DEBUG)->value())
  {
    auto ground_map = get<C::Ground_map>("background:ground_map");

    ground_map->for_each_vertex
      ([&](const Point& point)
       {
         m_core.draw_square (point.x() - xcamera, point.y(), 10);
       });

    ground_map->for_each_edge
      ([&](const Point& source, const Point& target, bool border)
       {
         m_core.draw_line (source.x() - xcamera, source.y(),
                           target.x() - xcamera, target.y(),
                           (border ? 255 : 0), 0, (border ? 0 : 255));
       });

    const std::string& id = get<C::String>("player:name")->value();
    auto path = request<C::Path>(id + ":path");
    if (path)
    {
      Point current = get<C::Position>(id + "_body:position")->value();
      m_core.draw_square (current.x() - xcamera, current.y(), 10, 0, 255, 0);

      for (std::size_t p = path->current(); p < path->size(); ++ p)
      {
        Point next = (*path)[p];
        m_core.draw_square (next.x() - xcamera, next.y(), 10, 0, 255, 0);
        m_core.draw_line (current.x() - xcamera, current.y(),
                          next.x() - xcamera, next.y(), 0, 255, 0);
        current = next;
      }

    }
  }
}

void Graphic::display_spin_loading()
{
  auto img = get<C::Image>(LOADING_SPIN__IMAGE);
  auto position = get<C::Position>(LOADING_SPIN__POSITION);
  Point p = position->value();

  int xmin = img->xmin();
  int ymin = img->ymin();
  int xmax = img->xmax();
  int ymax = img->ymax();

  Point screen_position = p - img->core().scaling * Vector(img->origin());

  m_core.draw (img->core(), xmin, ymin, (xmax - xmin), (ymax - ymin),
               screen_position.X(), screen_position.Y(),
               img->core().scaling * (xmax - xmin),
               img->core().scaling * (ymax - ymin));
}

} // namespace Sosage::System
