#include <Sosage/Component/Ground_map.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/profiling.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
#include <set>

namespace Sosage::Component
{

Ground_map::Ground_map (const std::string& id,
                        const std::string& file_name)
  : Base(id)
{
  debug ("Creating ground map " + file_name);
  Core::Graphic::Surface source = Core::Graphic::load_surface (file_name);

  Point size (Core::Graphic::width(source), Core::Graphic::height(source),
              WORLD);

  m_data = vector_2<Node> (size.x(GROUND),
                           size.y(GROUND));

  debug ("Size = " + std::to_string(m_data.width()) + " * " + std::to_string(m_data.height()));

  for (std::size_t x = 0; x < m_data.width(); ++ x)
    for (std::size_t y = 0; y < m_data.height(); ++ y)
    {
      Point point(x,y, GROUND);
      int xx = point.x(WORLD);
      int yy = point.y(WORLD);

      std::array<unsigned char, 3> color
        = Core::Graphic::get_color (source, xx, yy);

      if (color[0] == color[1] && color[0] == color[2])
      {
        m_data(x,y).z = config().world_depth * (1. - (color[0] / 255.)) + 1;
        m_data(x,y).target_x = -1;
        m_data(x,y).target_y = -1;
      }
      else
      {
        m_data(x,y).z = -1;
        m_data(x,y).target_x = 0;
        m_data(x,y).target_y = 0;
      }
    }

  std::vector<std::pair<int, int> > border;
  for (std::size_t x = 0; x < m_data.width(); ++ x)
    for (std::size_t y = 0; y < m_data.height(); ++ y)
      if (m_data(x,y).target_x == -1)
      {
        bool is_border = false;
        if (x > 0 && m_data(x-1,y).z == -1)
          is_border = true;
        else if (x < m_data.width() - 1 && m_data(x+1,y).z == -1)
          is_border = true;
        if (y > 0 && m_data(x,y-1).z == -1)
          is_border = true;
        else if (y < m_data.height() - 1 && m_data(x,y+1).z == -1)
          is_border = true;

        if (is_border)
          border.push_back (std::make_pair(x,y));
      }

  debug (std::to_string(border.size()) + " border point(s)");

  for (std::size_t x = 0; x < m_data.width(); ++ x)
    for (std::size_t y = 0; y < m_data.height(); ++ y)
      if (m_data(x,y).target_x != -1)
      {
        double dist_min = std::numeric_limits<double>::max();
        for (std::size_t i = 0; i < border.size(); ++ i)
        {
          double dist = Sosage::distance(x,y, border[i].first, border[i].second);
          if (dist < dist_min)
          {
            dist_min = dist;
            m_data(x,y).target_x = border[i].first;
            m_data(x,y).target_y = border[i].second;
          }
        }
      }

  // for (std::size_t y = 0; y < m_data.height(); y += m_data.height() / 50)
  // {
  //   std::cerr << "[";
  //   for (std::size_t x = 0; x < m_data.width(); x += m_data.width() / 50)
  //     if (m_data(x,y).target_x == -1)
  //       std::cerr << "#";
  //     else
  //       std::cerr << " ";
  //   std::cerr << "]" << std::endl;
  // }

  // {
  //         std::ofstream dbg ("dbg.ppm");
  //   dbg << "P3" << std::endl << m_data.width() << " " << m_data.height()
  //       << std::endl << "255" << std::endl;

  //   std::size_t nb = 0;
  //   for (std::size_t y = 0; y < m_data.height(); ++ y)
  //     for (std::size_t x = 0; x < m_data.width(); ++ x)
  //     {
  //       int r, g, b;
  //       if (m_data(x,y).target_x == -1)
  //       {
  //         bool is_border = false;
  //         if (x > 0 && m_data(x-1,y).target_x != -1)
  //           is_border = true;
  //         else if (x < m_data.width() - 1 && m_data(x+1,y).target_x != -1)
  //           is_border = true;
  //         if (y > 0 && m_data(x,y-1).target_x != -1)
  //           is_border = true;
  //         else if (y < m_data.height() - 1 && m_data(x,y+1).target_x != -1)
  //           is_border = true;

  //         if (is_border)
  //         {
  //           srand(x + m_data.width() * y);
  //           r = 64 + rand() % 128;
  //           g = 64 + rand() % 128;
  //           b = 64 + rand() % 128;
  //         }
  //         else
  //         {
  //           r = int(255 * (m_data(x,y).z / double(config().world_depth)));
  //           g = int(255 * (m_data(x,y).z / double(config().world_depth)));
  //           b = int(255 * (m_data(x,y).z / double(config().world_depth)));
  //         }
  //       }
  //       else
  //       {
  //         int xx = m_data(x,y).target_x;
  //         int yy = m_data(x,y).target_y;
  //         srand(xx + m_data.width() * yy);
  //         r = 64 + rand() % 128;
  //         g = 64 + rand() % 128;
  //         b = 64 + rand() % 128;
  //       }
  //       dbg << r << " " << g << " " << b << " ";
  //       if (nb ++ == 4)
  //       {
  //         dbg << std::endl;
  //         nb = 0;
  //       }
  //     }
  // }
}

void Ground_map::find_path (const Point& origin,
                            const Point& target,
                            std::vector<Point>& out)
{
  static Timer t ("Path finder");
  t.start();
  
  //  correct target position to reach true ground
  int x = target.x(GROUND);
  int y = target.y(GROUND);

  if (m_data(x,y).target_x != -1)
  {
    int xx = m_data(x,y).target_x;
    int yy = m_data(x,y).target_y;
    x = xx;
    y = yy;
  }

  int xorig = origin.x(GROUND);
  int yorig = origin.y(GROUND);
  
  if (m_data(xorig,yorig).target_x != -1)
  {
    int xx = m_data(xorig,yorig).target_x;
    int yy = m_data(xorig,yorig).target_y;
    xorig = xx;
    yorig = yy;
  }

  Point origin_corrected (xorig, yorig, GROUND);
  Point target_corrected (x, y, GROUND);
  
  // std::cerr << "Finding path from " << origin_corrected << " (corrected from "
  //           << origin << ") to " << target_corrected
  //           << " (corrected from " << target << ")" << std::endl;

  // Dijkstra

  Sort_by_distance sorter (m_data);
  std::set<std::pair<int, int>, Sort_by_distance> todo (sorter);
  
  std::size_t nb_nodes = 0;
  for (std::size_t x = 0; x < m_data.width(); ++ x)
    for (std::size_t y = 0; y < m_data.height(); ++ y)
      if (m_data(x,y).target_x == -1)
      {
        m_data(x,y).distance = std::numeric_limits<double>::max();
        m_data(x,y).prev_x = -1;
        m_data(x,y).prev_y = -1;
        if (x == xorig && y == yorig)
          m_data(x,y).distance = 0.;
        todo.insert (std::make_pair(x,y));
        ++ nb_nodes;
      }
  check (nb_nodes == todo.size(), "Node set wrongly formed");

  while (!todo.empty())
  {
    int current_x = todo.begin()->first;
    int current_y = todo.begin()->second;
    todo.erase (todo.begin());

    for (int x = -1; x <= 1; ++ x)
    {
      int xx = current_x + x;
      if (!(0 <= xx && xx < m_data.width()))
        continue;

      for (int y = -1; y <= 1; ++ y)
      {
        if (x == 0 && y == 0)
          continue;
        
        int yy = current_y + y;
        if (!(0 <= yy && yy < m_data.height()))
          continue;

        auto iter = todo.find(std::make_pair(xx,yy));
        if (iter == todo.end())
          continue;

        double dist = m_data(current_x,current_y).distance
          + distance(current_x, current_y, xx, yy);
        if (dist < m_data(xx, yy).distance)
        {
          todo.erase(iter);
          m_data(xx,yy).distance = dist;
          m_data(xx,yy).prev_x = current_x;
          m_data(xx,yy).prev_y = current_y;
          todo.insert (std::make_pair (xx, yy));
        }
      }
    }
  }
  
  std::vector<Point> path;
  while (!(x == xorig && y == yorig))
  {
    check (x != -1 && y != -1, "Node has no parent");
    path.push_back (Point (x, y, GROUND));
    int next_x = m_data(x,y).prev_x;
    int next_y = m_data(x,y).prev_y;
    x = next_x;
    y = next_y;
  }
  path.push_back (Point (x, y, GROUND));

  std::reverse(path.begin(), path.end());
  
  // Filter
  {
    std::vector<bool> insert (path.size(), false);
    insert.back() = true;
    std::queue<std::pair<std::size_t, std::size_t> > todo;
    todo.push(std::make_pair (0, path.size()));
    while (!todo.empty())
    {
      std::size_t first = todo.front().first;
      std::size_t last = todo.front().second;
      todo.pop();

      Vector v (path[first], path[last-1]);
      v.normalize();

      double dist_max = 0;
      std::size_t farthest;
      for (std::size_t i = first+1; i < last; ++ i)
      {
        Vector vh (path[first], path[i]);
        double hypo = vh.length();
        vh.normalize();
        double angle = std::acos(v * vh);
        double sin = std::sin(angle);
        double dist = hypo * sin;
        if (dist > dist_max)
        {
          dist_max = dist;
          farthest = i;
        }
      }

      if (dist_max > config().ground_map_scaling)
      {
        insert[farthest] = true;
        todo.push(std::make_pair(first, farthest));
        todo.push(std::make_pair(farthest, last));
      }
    }

    for (std::size_t i = 0; i < path.size(); ++ i)
      if (insert[i])
        out.push_back (path[i]);
  }

  t.stop();
}

double Ground_map::z_at_point (const Point& p) const
{
  const Node& n = m_data(p.x(GROUND), p.y(GROUND));
  if (n.target_x != -1)
    return m_data(n.target_x, n.target_y).z;
  // else
  return n.z;
}

} // namespace Sosage::Component
