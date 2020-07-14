/*
  [src/Sosage/Component/Ground_map.cpp]
  Ground limits, local depth and path finding.

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

#include <Sosage/Component/Ground_map.h>
#include <Sosage/Utils/profiling.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
#include <set>

namespace Sosage::Component
{

Ground_map::Ground_map (const std::string& id,
                        const std::string& file_name,
                        int front_z, int back_z)
  : Base(id), m_front_z (front_z), m_back_z (back_z)
{
  SOSAGE_TIMER_START(Ground_map__Ground_map);
  
  m_image = Core::Graphic::load_surface (file_name);

  int width = Core::Graphic::width(m_image);
  int height = Core::Graphic::height(m_image);

  // Build border of ground area
  
  std::map<Point, GVertex> map_p2v;

  for (int x = 0; x < width - 1; ++ x)
    for (int y = 0; y < height - 1; ++ y)
    {
      std::array<unsigned char, 3> c
        = Core::Graphic::get_color (m_image, x, y);
      std::array<unsigned char, 3> c_right
        = Core::Graphic::get_color (m_image, x+1, y);
      std::array<unsigned char, 3> c_down
        = Core::Graphic::get_color (m_image, x, y+1);

      bool g = (c[0] == c[1] && c[0] == c[2]);
      bool g_right = (c_right[0] == c_right[1] && c_right[0] == c_right[2]);
      bool g_down = (c_down[0] == c_down[1] && c_down[0] == c_down[2]);

      if (g != g_right)
      {
        unsigned char red = (g ? c[0] : c_right[0]);
        
        GVertex source = add_vertex (map_p2v, Point (x + 0.5, y - 0.5), red);
        GVertex target = add_vertex (map_p2v, Point (x + 0.5, y + 0.5), red);

        m_graph.add_edge (source, target);
      }
      if (g != g_down)
      {
        unsigned char red = (g ? c[0] : c_down[0]);
        
        GVertex source = add_vertex (map_p2v, Point (x - 0.5, y + 0.5), red);
        GVertex target = add_vertex (map_p2v, Point (x + 0.5, y + 0.5), red);

        m_graph.add_edge (source, target);
      }
    }

  // Simplify border

  std::set<GVertex, std::function<bool(const GVertex&, const GVertex&)> >
    todo ([&] (const GVertex& a, const GVertex& b) -> bool
          {
            double da = deviation(a);
            double db = deviation(b);
            if (da == db)
              return a < b;
            return da < db;
          });
  
  for (GVertex v : m_graph.vertices())
  {
    if (deviation(v) < Config::boundary_precision)
      todo.insert (v);
  }

  while (!todo.empty())
  {
    GVertex v = *todo.begin();
    todo.erase(todo.begin());

    GVertex n0 = m_graph.incident_vertex(v, 0);
    GVertex n1 = m_graph.incident_vertex(v, 1);

    auto it = todo.find(n0);
    if (it != todo.end())
      todo.erase (it);
    it = todo.find(n1);
    if (it != todo.end())
      todo.erase (it);

    m_graph.delete_edge(m_graph.incident_edge(v, 0));
    m_graph.delete_edge(m_graph.incident_edge(v, 1));

    m_graph.add_edge (n0, n1);

    if (deviation(n0) < Config::boundary_precision)
      todo.insert (n0);
    if (deviation(n1) < Config::boundary_precision)
      todo.insert (n1);
  }

  m_graph.validity();
  m_graph.clean();
  m_graph.validity();

  debug("Edges = " + std::to_string(m_graph.num_edges()));
  
  for (auto it0 = m_graph.vertices().begin(); it0 != m_graph.vertices().end(); ++ it0)
  {
    GVertex v0 = *it0;
    auto it1 = it0;
    ++ it1;
    for (; it1 != m_graph.vertices().end(); ++ it1)
    {
      GVertex v1 = *it1;
      if (m_graph.is_edge(v0, v1))
        continue;
      Segment seg (m_graph[v0].point, m_graph[v1].point);
      if (intersects_border
          (m_graph, seg,
           [&](const GEdge& e) -> bool
           {
             return (m_graph.edge_has_vertex(e, v0) ||
                     m_graph.edge_has_vertex(e, v1));
           }))
        continue;

      Point mid = midpoint (m_graph[v0].point, m_graph[v1].point);
      if (!is_ground_point(mid))
        continue;

      GEdge e = m_graph.add_edge(v0, v1);
      m_graph[e].border = false;
    }
  }
  debug("Edges = " + std::to_string(m_graph.num_edges()));
  
  m_latest_graph = m_graph;
  
  SOSAGE_TIMER_STOP(Ground_map__Ground_map);
}

void Ground_map::find_path (Point origin,
                            Point target,
                            std::vector<Point>& out)
{
  SOSAGE_TIMER_START(Ground_map__find_path);

  m_latest_graph = m_graph;

  GVertex vorigin = Graph::null_vertex();
  GVertex vtarget = Graph::null_vertex();
  GEdge eorigin = Graph::null_edge();
  GEdge etarget = Graph::null_edge();
  std::set<std::pair<GVertex, GVertex>, Compare_ordered_pair> to_add;
  
  if (!is_ground_point(origin))
  {
    Neighbor_query query = closest_simplex(origin);
    origin = query.point;
    if (query.edge == Graph::null_edge())
      vorigin = query.vertex;
    else
      eorigin = query.edge;
  }
  if (!is_ground_point(target))
  {
    Neighbor_query query = closest_simplex(target);
    target = query.point;
    if (query.edge == Graph::null_edge())
      vtarget = query.vertex;
    else
      etarget = query.edge;
  }

  if (eorigin != Graph::null_edge() && eorigin == etarget) // moving along the same line
  {
    out.push_back (target);
    return;
  }

  if (eorigin != Graph::null_edge())
  {
    vorigin = m_latest_graph.add_vertex(origin);
    GVertex v0 = m_latest_graph.source(eorigin);
    GVertex v1 = m_latest_graph.target(eorigin);
    m_latest_graph.delete_edge(eorigin);
    to_add.insert (std::make_pair (vorigin, v0));
    to_add.insert (std::make_pair (vorigin, v1));
  }
  else if (vorigin == Graph::null_vertex())
    vorigin = m_latest_graph.add_vertex(origin);
    
  if (etarget != Graph::null_edge())
  {
    vtarget = m_latest_graph.add_vertex(target);
    GVertex v0 = m_latest_graph.source(etarget);
    GVertex v1 = m_latest_graph.target(etarget);
    m_latest_graph.delete_edge(etarget);
    to_add.insert (std::make_pair (vtarget, v0));
    to_add.insert (std::make_pair (vtarget, v1));
  }
  else if (vtarget == Graph::null_vertex())
    vtarget = m_latest_graph.add_vertex(target);

  Segment segment (origin, target);
  
  // If no  border intersected, straight line is fine
  if (!intersects_border
      (m_latest_graph, segment,
       [&](const GEdge& e) -> bool
       {
         return (m_latest_graph.edge_has_vertex(e, vorigin)
                 || m_latest_graph.edge_has_vertex(e, vtarget));
       }))
  {
    // Additional check if we join 2 borders through a hole
    Point mid = midpoint (origin, target);
    if (is_ground_point(mid))
    {
      out.push_back (target);
      m_latest_graph = m_graph;
      SOSAGE_TIMER_STOP(Ground_map__find_path);
      return;
    }
  }

  // Insert new edges
  for (GVertex v : m_latest_graph.vertices())
  {
    if (v == vorigin || v == vtarget)
      continue;

    for (GVertex n : { vorigin, vtarget })
    {
      Point mid = midpoint (m_latest_graph[v].point, m_latest_graph[n].point);
      if (!is_ground_point(mid))
        continue;
        
      Segment seg (m_latest_graph[v].point, m_latest_graph[n].point);
      if (intersects_border
          (m_latest_graph, seg,
           [&](const GEdge& e) -> bool
           {
             return ((e == eorigin || e == etarget) || m_latest_graph.edge_has_vertex(e, v));
           }))
        continue;

      to_add.insert (std::make_pair(v, n));
    }
  }

  for (const auto& p : to_add)
  {
    GEdge e = m_latest_graph.add_edge(p.first, p.second);
    m_latest_graph[e].border = false;
  }

  m_latest_graph.clean();
  
  // Dijkstra
  m_latest_graph.validity();

  shortest_path(vorigin, vtarget, out);

  SOSAGE_TIMER_STOP(Ground_map__find_path);
}

double Ground_map::z_at_point (const Point& p) const
{
  std::array<unsigned char, 3> c = Core::Graphic::get_color (m_image, p.x(), p.y());

  unsigned char red = 0;
  if (c[0] == c[1] && c[0] == c[2])
    red = c[0];
  else
  {
    Neighbor_query query = closest_simplex (p);

    if (query.edge == Graph::null_edge()) // point case
      red = m_graph[query.vertex].red;
    else // segment case
    {
      double dsource = distance (query.point, m_graph[m_graph.source(query.edge)].point);
      double dtarget = distance (query.point, m_graph[m_graph.target(query.edge)].point);

      red = (m_graph[m_graph.source(query.edge)].red * dtarget
             + m_graph[m_graph.target(query.edge)].red * dsource)
        / (dsource + dtarget);
    }
  }
  
  return m_back_z + (m_front_z - m_back_z) * (1. - (red / 255.));
}

bool Ground_map::is_ground_point (const Point& p) const
{
  std::array<unsigned char, 3> c = Core::Graphic::get_color (m_image, p.x(), p.y());
  return (c[0] == c[1] && c[0] == c[2]);
}

Ground_map::Neighbor_query Ground_map::closest_simplex (const Point& p) const
{
  double min_dist = std::numeric_limits<double>::max();
  GVertex vertex = Graph::null_vertex();
  GEdge edge = Graph::null_edge();
  Point point = p;
  
  for (GVertex v : m_graph.vertices())
  {
    double dist = distance (p, m_graph[v].point);
    if (dist < min_dist)
    {
      point = m_graph[v].point;
      min_dist = dist;
      vertex = v;
    }
  }

  for (GEdge e : m_graph.edges())
  {
    GVertex vsource = m_graph.source(e);
    GVertex vtarget = m_graph.target(e);
    const Point& psource = m_graph[vsource].point;
    const Point& ptarget = m_graph[vtarget].point;
    Segment seg (psource, ptarget);
      
    Point proj;
    bool does_project;
    std::tie (proj, does_project) = seg.projection (p);
    if (does_project)
    {
      double dist = distance (p, proj);
      if (dist < min_dist)
      {
        point = proj;
        min_dist = dist;
        vertex = Graph::null_vertex();
        edge = e;
      }
    }
  }
  
  return Neighbor_query (vertex, edge, min_dist, point);
}

void Ground_map::shortest_path (GVertex vorigin, GVertex vtarget,
                                std::vector<Point>& out)
{
  // Djikstra
  std::priority_queue<std::pair<double, GVertex> > todo;

  std::vector<GVertex> parent (m_latest_graph.vertices().size(), Graph::null_vertex());
  
  for (GVertex v : m_latest_graph.vertices())
  {
    double dist = std::numeric_limits<double>::max();
    if (v == vorigin)
      dist = 0;
    m_latest_graph[v].dist = dist;
    todo.push (std::make_pair(-dist, v));
  }

  while (!todo.empty())
  {
    double current_dist = -todo.top().first;
    GVertex current = todo.top().second;
    todo.pop();

    if (m_latest_graph[current].dist < current_dist) // outdated item
      continue;
    
    for (std::size_t i = 0; i < m_latest_graph.incident_edges(current).size(); ++ i)
    {
      GVertex next = m_latest_graph.incident_vertex(current,i);
      double dist = current_dist + distance (m_latest_graph[current].point,
                                             m_latest_graph[next].point);
      if (dist < m_latest_graph[next].dist)
      {
        m_latest_graph[next].dist = dist;
        parent[std::size_t(next)] = current;
        todo.push (std::make_pair (-dist, next));
      }
    }
  }
  
  std::vector<Point> path;
  while (vtarget != vorigin)
  {
    check (parent[vtarget] != Graph::null_vertex(), "Node has no parent");
    path.push_back (m_latest_graph[vtarget].point);
    vtarget = parent[std::size_t(vtarget)];
  }
  std::copy (path.rbegin(), path.rend(),
             std::back_inserter (out));
}

bool Ground_map::intersects_border (const Graph& g,
                                    const Segment& seg,
                                    const std::function<bool(const GEdge&)>& condition) const
{
  for (GEdge e : g.edges())
  {
    if (!g[e].border || condition(e))
      continue;
    Segment eseg (g[g.source(e)].point,
                  g[g.target(e)].point);
    if (intersect(seg, eseg))
      return true;
  }
  return false;
}

} // namespace Sosage::Component
