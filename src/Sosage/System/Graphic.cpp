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
#include <Sosage/Utils/conversions.h>

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
  auto iw = get<C::Int>("Window", "width");
  auto ih = get<C::Int>("Window", "height");
  int w = iw->value();
  int h = ih->value();
  m_core.init (w, h,
               value<C::Boolean>("Window", "fullscreen"));
  iw->set(w);
  ih->set(h);

}

void Graphic::run()
{
  SOSAGE_TIMER_START(System_Graphic__run);
  if (auto name = request<C::String>("Game", "name"))
  {
    m_core.update_window (locale(name->value()), value<C::String>("Icon", "filename"));
    remove ("Game", "name");
  }

  if (request<C::String>("Game", "new_room"))
  {
    m_core.clear_managers();
    run_loading();
    SOSAGE_TIMER_STOP(System_Graphic__run);
    return;
  }

  if (receive ("Window", "rescaled"))
    m_core.update_view ();
  if (receive ("Window", "toggle_fullscreen"))
    m_core.toggle_fullscreen (value<C::Boolean>("Window", "fullscreen"));
  if (receive ("Fake_touchscreen", "enable"))
    m_core.toggle_cursor(true);
  if (receive ("Fake_touchscreen", "disable"))
    m_core.toggle_cursor(false);

  m_core.begin();

  const Point& camera = value<C::Absolute_position>(CAMERA__POSITION);

  using Image_with_info = std::tuple<C::Image_handle, double, double, double, double, double>;
  static std::vector<Image_with_info> to_display;

  double limit_width = Config::world_width;
  double limit_height = Config::world_height;

  for (const auto& e : components("image"))
    if (auto img = C::cast<C::Image>(e))
    {
      if (!img->on())
        continue;
      if (status()->is (LOCKED) &&
          img->entity() == "Cursor")
        continue;

      auto position = get<C::Position>(img->entity() , "position");
      Point p = position->value();
      double zoom = 1.;
      if (!position->is_interface())
      {
        p = p - camera;
        zoom = value<C::Double>(CAMERA__ZOOM);
      }

      int xmin = img->xmin();
      int ymin = img->ymin();
      int xmax = img->xmax();
      int ymax = img->ymax();

      Point screen_position = p - img->core().scaling * Vector(img->origin());

      double xmin_target = zoom * screen_position.x();
      double ymin_target = zoom * screen_position.y();
      double xmax_target = zoom * (screen_position.x() + img->core().scaling * (xmax - xmin));
      double ymax_target = zoom * (screen_position.y() + img->core().scaling * (ymax - ymin));

      // Skip out of boundaries images
      if ((xmax_target < 0 || xmin_target > limit_width)
          && (ymax_target < 0 || ymin_target > limit_height))
          continue;

      to_display.emplace_back (img, xmin_target, ymin_target, xmax_target, ymax_target, zoom);
    }

  std::sort (to_display.begin(), to_display.end(),
             [](const Image_with_info& a, const Image_with_info& b) -> bool
             {
               return (std::get<0>(a)->z() < std::get<0>(b)->z());
             });

  for (auto& td : to_display)
  {
    auto img = std::get<0>(td);
    int xmin = img->xmin();
    int ymin = img->ymin();
    int xmax = img->xmax();
    int ymax = img->ymax();

    double xmin_target = std::get<1>(td);
    double ymin_target = std::get<2>(td);
    double xmax_target = std::get<3>(td);
    double ymax_target = std::get<4>(td);
    double zoom = std::get<5>(td);

    // Cut if image goes beyond boundaries
    if (xmin_target < 0)
    {
      xmin -= round(xmin_target / (img->core().scaling * zoom));
      xmin_target = 0;
    }
    if (ymin_target < 0)
    {
      ymin -= round(ymin_target / (img->core().scaling * zoom));
      ymin_target = 0;
    }
    if (xmax_target > limit_width)
    {
      xmax -= round((xmax_target - limit_width) / (img->core().scaling * zoom));
      xmax_target = limit_width;
    }
    if (ymax_target > limit_height)
    {
      ymax -= round((ymax_target - limit_height) / (img->core().scaling * zoom));
      ymax_target = limit_height;
    }

    int width = xmax - xmin;
    int height = ymax - ymin;

    double width_target = xmax_target - xmin_target;
    double height_target = ymax_target - ymin_target;

    m_core.draw (img->core(), xmin, ymin, width, height,
                 round(xmin_target), round(ymin_target),
                 round(width_target), round(height_target));
  }

  if (value<C::Boolean>(GAME__DEBUG))
  {
    if (auto ground_map = request<C::Ground_map>("Background", "ground_map"))
    {
        ground_map->for_each_vertex
        ([&](const Point& point)
        {
          Point p = point - camera;
          m_core.draw_square (p.X(), p.Y(), 10);
        });

        ground_map->for_each_edge
        ([&](const Point& source, const Point& target, bool border)
        {
          Point s = source - camera;
          Point t = target - camera;
          m_core.draw_line (s.X(), s.Y(), t.X(), t.Y(),
                            (border ? 255 : 0), 0, (border ? 0 : 255));
        });

        for (auto c : components("path"))
          if (auto path = C::cast<C::Path>(c))
          {
            Point current = value<C::Position>(path->entity() + "_body", "position") - camera;
            m_core.draw_square (current.X(), current.Y(), 10, 0, 255, 0);

            for (std::size_t p = path->current(); p < path->size(); ++ p)
            {
              Point next = (*path)[p] - camera;
            m_core.draw_square (next.X(), next.Y(), 10, 0, 255, 0);
            m_core.draw_line (current.X(), current.Y(),
                              next.X(), next.Y(), 0, 255, 0);
            current = next;
          }
        }
    }

    for (const auto& td: to_display)
    {
      auto img = std::get<0>(td);
      auto pos = img->entity().find("_label");
      if (pos == std::string::npos)
        continue;
      std::string id (img->entity().begin(), img->entity().begin() + pos);

      if (!request<C::String>(id , "name"))
        continue;

      auto view = value<C::Position>(id , "view") - camera;
      double reach_factor = value<C::Double>(id, "reach_factor", 1.);

      m_core.draw_rectangle (view.X(), view.Y(),
                             2 * reach_factor * Config::object_reach_x, 2 * reach_factor * Config::object_reach_y,
                             255, 0, 0, 16);
    }

  }

  to_display.clear();

  m_core.end();

  SOSAGE_TIMER_STOP(System_Graphic__run);
}

void Graphic::run_loading()
{
  m_core.begin();

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
