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


#include <Sosage/Component/Animation.h>
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
  auto iw = get<C::Int>("Window:width");
  auto ih = get<C::Int>("Window:height");
  int w = iw->value();
  int h = ih->value();
  m_core.init (w, h,
               get<C::Boolean>("Window:fullscreen")->value());
  iw->set(w);
  ih->set(h);

}

void Graphic::run()
{
  if (auto name = request<C::String>("Game:name"))
  {
    m_core.update_window (name->value(), get<C::String>("Icon:filename")->value());
    remove ("Game:name");
  }

  if (request<C::String>("Game:new_room"))
  {
    m_core.clear_managers();
    run_loading();
    return;
  }

  if (receive ("Window:rescaled"))
    m_core.update_view (get<C::Int>("Interface:width")->value(),
                        get<C::Int>("Interface:height")->value());
  if (receive ("Window:toggle_fullscreen"))
    m_core.toggle_fullscreen (get<C::Boolean>("Window:fullscreen")->value());

  std::vector<C::Image_handle> images;

  m_core.begin(get<C::Int>("Interface:width")->value(),
               get<C::Int>("Interface:height")->value());

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

  int interface_width = get<C::Int>("Interface:width")->value();
  int interface_height = get<C::Int>("Interface:height")->value();

  for (const auto& img : images)
  {
    if (img->on())
    {
      if (status->value() == LOCKED &&
           img->entity() == "Cursor")
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

      int xmin_target = screen_position.X();
      int ymin_target = screen_position.y();
      int xmax_target = xmin_target + int(img->core().scaling * (xmax - xmin));
      int ymax_target = ymin_target + int(img->core().scaling * (ymax - ymin));

      int limit_width = Config::world_width;
      if (position->absolute())
        limit_width += interface_width;
      int limit_height = Config::world_height;
      if (position->absolute())
        limit_height += interface_height;

      // Skip out of boundaries images
      if ((xmax_target < 0 || xmin_target > limit_width)
          && (ymax_target < 0 || ymin_target > limit_height))
        continue;

      // Cut if image goes beyond boundaries
      if (xmin_target < 0)
      {
        xmin -= xmin_target / img->core().scaling;
        xmin_target = 0;
      }
      if (ymin_target < 0)
      {
        ymin -= ymin_target / img->core().scaling;
        ymin_target = 0;
      }
      if (xmax_target > limit_width)
      {
        xmax -= (xmax_target - limit_width) / img->core().scaling;
        xmax_target = limit_width;
      }
      if (ymax_target > limit_height)
      {
        ymax -= (ymax_target - limit_height) / img->core().scaling;
        ymax_target = limit_height;
      }

      int width = xmax - xmin;
      int height = ymax - ymin;

      int width_target = xmax_target - xmin_target;
      int height_target = ymax_target - ymin_target;

      m_core.draw (img->core(), xmin, ymin, width, height,
                   xmin_target, ymin_target,
                   width_target, height_target);
    }
  }

  if (get<C::Boolean>(GAME__DEBUG)->value())
  {
    if (auto ground_map = request<C::Ground_map>("Background:ground_map"))
    {
        ground_map->for_each_vertex
        ([&](const Point& point)
        {
           m_core.draw_square (int(point.x() - xcamera), point.Y(), 10);
        });

        ground_map->for_each_edge
        ([&](const Point& source, const Point& target, bool border)
        {
           m_core.draw_line (int(source.x() - xcamera), source.Y(),
                            int(target.x() - xcamera), target.Y(),
                            (border ? 255 : 0), 0, (border ? 0 : 255));
        });

        const std::string& id = get<C::String>("Player:name")->value();
        auto path = request<C::Path>(id + ":path");
        if (path)
        {
          Point current = get<C::Position>(id + "_body:position")->value();
          m_core.draw_square (int(current.x() - xcamera), current.Y(), 10, 0, 255, 0);

          for (std::size_t p = path->current(); p < path->size(); ++ p)
          {
            Point next = (*path)[p];
            m_core.draw_square (int(next.x() - xcamera), next.Y(), 10, 0, 255, 0);
            m_core.draw_line (int(current.x() - xcamera), current.Y(),
                              int(next.x() - xcamera), next.Y(), 0, 255, 0);
            current = next;
          }
        }
    }
  }
}

void Graphic::run_loading()
{
  m_core.begin(get<C::Int>("Interface:width")->value(),
               get<C::Int>("Interface:height")->value());

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
               int(img->core().scaling * (xmax - xmin)),
               int(img->core().scaling * (ymax - ymin)));

  m_core.end();
}

} // namespace Sosage::System
