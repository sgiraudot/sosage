#include <Sosage/Component/Ground_map.h>

#include <algorithm>
#include <cassert>
#include <set>

namespace Sosage::Component
{

Ground_map::Ground_map (const std::string& file_name)
{
  std::cerr << "Creating ground map " << file_name << std::endl;
  Core::Image source = Core::load_image (file_name);

  Point size (Core::width(source), Core::height(source),
              CAMERA);

  m_data = vector_2<Node> (size.x(GROUND),
                           size.y(GROUND));

  std::cerr << "Size = " << m_data.width() << " * " << m_data.height() << std::endl;

  for (std::size_t x = 0; x < m_data.width(); ++ x)
    for (std::size_t y = 0; y < m_data.height(); ++ y)
    {
      Point point(x,y, GROUND);
      int xx = point.x(CAMERA);
      int yy = point.y(CAMERA);

      std::array<unsigned char, 3> color
        = Core::get_color (source, xx, yy);

      if (color[0] == 0 && color[1] == 0 && color[2] == 255) // non-ground
      {
        m_data(x,y).z = -1;
        m_data(x,y).target_x = 0;
        m_data(x,y).target_y = 0;
      }
      else
      {
        m_data(x,y).z = ((color[0] + color[1] + color[2]) / (3. * 255.));
        m_data(x,y).target_x = -1;
        m_data(x,y).target_y = -1;
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

  std::cerr << border.size() << " border point(s)" << std::endl;

  for (std::size_t x = 0; x < m_data.width(); ++ x)
    for (std::size_t y = 0; y < m_data.height(); ++ y)
      if (m_data(x,y).target_x != -1)
      {
        double dist_min = std::numeric_limits<double>::max();
        for (std::size_t i = 0; i < border.size(); ++ i)
        {
          double dist = distance(x,y, border[i].first, border[i].second);
          if (dist < dist_min)
          {
            dist_min = dist;
            m_data(x,y).target_x = border[i].first;
            m_data(x,y).target_y = border[i].second;
          }
        }
      }

  for (std::size_t y = 0; y < m_data.height(); y += m_data.height() / 50)
  {
    std::cerr << "[";
    for (std::size_t x = 0; x < m_data.width(); x += m_data.width() / 50)
      if (m_data(x,y).target_x == -1)
        std::cerr << "#";
      else
        std::cerr << " ";
    std::cerr << "]" << std::endl;
  }
}

void Ground_map::find_path (const Point& origin,
                            const Point& target,
                            std::vector<Point>& out)
{
  //  correct target position to reach true ground
  int x = target.x(GROUND);
  int y = target.y(GROUND);
  std::cerr << x << "*" << y << std::endl;

  if (m_data(x,y).target_x != -1)
  {
    int xx = m_data(x,y).target_x;
    int yy = m_data(x,y).target_y;
    x = xx;
    y = yy;
  }

  Point corrected (x, y, GROUND);

  std::cerr << "Finding path from " << origin << " to " << corrected
            << " (corrected from " << target << ")" << std::endl;

  // shortest path
//  1  function Dijkstra(Graph, source):
//  2
//  3      create vertex set Q
//  4
//  5      for each vertex v in Graph:             
//  6          dist[v] ← INFINITY                  
//  7          prev[v] ← UNDEFINED                 
//  8          add v to Q                      
// 10      dist[source] ← 0                        
// 11      
// 12      while Q is not empty:
// 13          u ← vertex in Q with min dist[u]    
// 14                                              
// 15          remove u from Q 
// 16          
// 17          for each neighbor v of u:           // only v that are still in Q
// 18              alt ← dist[u] + length(u, v)
// 19              if alt < dist[v]:               
// 20                  dist[v] ← alt 
// 21                  prev[v] ← u 
// 22
// 23      return dist[], prev[]

  Sort_by_distance sorter (m_data);
  std::set<std::pair<int, int>, Sort_by_distance> todo (sorter);
  
  int xorig = origin.x(GROUND);
  int yorig = origin.y(GROUND);
  
  for (std::size_t x = 0; x < m_data.width(); ++ x)
    for (std::size_t y = 0; y < m_data.height(); ++ y)
      if (m_data(x,y).target_x == -1)
      {
        m_data(x,y).distance = std::numeric_limits<int>::max();
        m_data(x,y).prev_x = -1;
        m_data(x,y).prev_y = -1;
        if (x == xorig && y == yorig)
          m_data(x,y).distance = 0;
        todo.insert (std::make_pair(x,y));
      }

  while (!todo.empty())
  {
    int current_x = todo.begin()->first;
    int current_y = todo.begin()->second;
//    std::cerr << current_x << "*" << current_y << " (" << m_data(current_x, current_y).distance << ")" << std::endl;
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

        int dist = m_data(current_x,current_y).distance + 100 * std::abs(x) + std::abs(y);
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

  while (!(x == xorig && y == yorig))
  {
    assert (x != -1 && y != -1);
    out.push_back (Point (x, y, GROUND));
    int next_x = m_data(x,y).prev_x;
    int next_y = m_data(x,y).prev_y;
    x = next_x;
    y = next_y;
  }

  std::reverse(out.begin(), out.end());
}

} // namespace Sosage::Component
