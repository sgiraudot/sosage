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
#include <Sosage/Utils/Asset_manager.h>
#include <Sosage/Utils/binary_io.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Utils/profiling.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
#include <set>

#define SOSAGE_DEBUG_GROUND_MAP
#ifdef SOSAGE_DEBUG_GROUND_MAP
#  define debug_gm debug
#else
#  define debug_gm if(false) std::cerr
#endif

namespace Sosage::Component
{

double Ground_map::deviation (Ground_map::GVertex v) const
{
  if (m_graph.incident_edges(v).size() != 2)
    return std::numeric_limits<double>::max();

  GVertex v0 = m_graph.incident_vertex(v, 0);
  GVertex v1 = m_graph.incident_vertex(v, 1);

  Line l (m_graph[v0].point, m_graph[v1].point);
  Point proj = l.projection (m_graph[v].point);
  return distance (m_graph[v].point, proj);
}

Ground_map::Neighbor_query::operator bool() const
{
  return (vertex != Graph::null_vertex() || edge != Graph::null_edge());
}

Ground_map::GVertex Ground_map::add_vertex (std::map<Point, Ground_map::GVertex>& map_p2v,
                                            const Point& p, const unsigned char& red)
{
  auto it = map_p2v.insert (std::make_pair (p, Graph::null_vertex()));
  if (it.second)
  {
    GVertex v = m_graph.add_vertex ({p, red});
    it.first->second = v;
  }
  return it.first->second;
}

Ground_map::Ground_map (const std::string& entity, const std::string& component,
                        const std::string& file_name,
                        int front_z, int back_z,
                        const std::function<void()>& callback)
  : Base(entity, component), m_front_z (front_z), m_back_z (back_z)
{
  SOSAGE_TIMER_START(Ground_map__Ground_map);
  
  m_image = Core::Graphic::load_surface (file_name);

  int width = m_image->w;
  int height = m_image->h;
  m_radius = int(distance(0, 0, width, height));

  if (Asset_manager::packaged())
  {
    std::string graph_file = file_name;
    graph_file.resize (graph_file.size() - 3);
    graph_file += "graph";
    read(graph_file);
  }
  else
    build_graph(callback);
  
  m_latest_graph = m_graph;
  
  SOSAGE_TIMER_STOP(Ground_map__Ground_map);
}

void Ground_map::build_graph (const std::function<void()>& callback)
{
  int width = m_image->w;
  int height = m_image->h;

  // Build border of ground area
  std::map<Point, GVertex> map_p2v;

  for (int x = -1; x < width; ++ x)
  {
    for (int y = -1; y < height; ++ y)
    {
      std::array<unsigned char, 3> c = { 0, 0, 255 };
      if (x != -1 && y != -1)
        c = Core::Graphic::get_color (m_image, x, y);

      std::array<unsigned char, 3> c_right = { 0, 0, 255 };
      if (x != width-1 && y != -1)
        c_right = Core::Graphic::get_color (m_image, x+1, y);

      std::array<unsigned char, 3> c_down = { 0, 0, 255 };
      if (y != height-1 && x != -1)
        c_down = Core::Graphic::get_color (m_image, x, y+1);

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
    callback();
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
    callback();
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
    callback();
  }

  m_graph.validity();
  m_graph.clean();
  m_graph.validity();

  debug << "Edges = " << m_graph.num_edges() << std::endl;

  // Add edges between vertices
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

      // Snap to close vertices along the way
      std::vector<GVertex> vertices_along = { v0, v1 };
      for (auto v : m_graph.vertices())
      {
        if (v == v0 || v == v1)
          continue;
        auto project = seg.projection(m_graph[v].point);
        if (!project.second)
          continue;

        if (distance(m_graph[v].point, project.first) > snapping_dist * 2)
          continue;

        vertices_along.push_back(v);
      }
      std::sort (vertices_along.begin(), vertices_along.end(),
                 [&](const GVertex& a, const GVertex& b) -> bool
      {
        return seg.projected_coordinate(m_graph[a].point)
            < seg.projected_coordinate(m_graph[b].point);
      });

      for (std::size_t i = 0; i < vertices_along.size() - 1; ++ i)
      {
        GVertex a = vertices_along[i];
        GVertex b = vertices_along[i+1];
        if (m_graph.is_edge(a, b))
          continue;
        GEdge e = m_graph.add_edge(a, b);
        m_graph[e].border = false;
      }
    }
    callback();
  }
  debug << "Edges = " << m_graph.num_edges() << std::endl;
}

void Ground_map::write (const std::string& filename)
{
  std::ofstream ofile (filename, std::ios::binary);

  auto nb_vertices = (unsigned short)(m_graph.vertices().size());
  binary_write (ofile, nb_vertices);

  for (GVertex v : m_graph.vertices())
  {
    binary_write (ofile, m_graph[v].point.X());
    binary_write (ofile, m_graph[v].point.Y());
    binary_write (ofile, m_graph[v].red);
  }

  auto nb_edges = (unsigned short)(m_graph.edges().size());
  binary_write (ofile, nb_edges);

  for (GEdge e : m_graph.edges())
  {
    auto s = (unsigned short)(m_graph.source(e));
    binary_write (ofile, s);
    auto t = (unsigned short)(m_graph.target(e));
    binary_write (ofile, t);
    auto b = (unsigned char)(m_graph[e].border);
    binary_write (ofile, b);
  }
}

void Ground_map::read (const std::string& filename)
{
  Asset asset = Asset_manager::open(filename);
  auto nb_vertices = asset.binary_read<unsigned short>();
  for (unsigned short v = 0; v < nb_vertices; ++ v)
  {
    auto x = asset.binary_read<int>();
    auto y = asset.binary_read<int>();
    auto red = asset.binary_read<unsigned char>();
    m_graph.add_vertex ({Point(x,y), red});
  }

  auto nb_edges = asset.binary_read<unsigned short>();
  for (unsigned short e = 0; e < nb_edges; ++ e)
  {
    auto s = asset.binary_read<unsigned short>();
    auto t = asset.binary_read<unsigned short>();
    auto b = bool(asset.binary_read<unsigned char>());
    GEdge edge = m_graph.add_edge (s, t);
    m_graph[edge].border = b;
  }
  asset.close();
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

  debug_gm << "Finding path from " << origin << " to " << target << std::endl;
  {
    Neighbor_query query = closest_simplex(origin);
    if (!is_ground_point(origin) || query.dist < snapping_dist)
    {
      origin = query.point;
      if (query.edge == Graph::null_edge())
      {
        vorigin = query.vertex;
        debug_gm << "Snap origin to closest vertex " << query.point << std::endl;
      }
      else
      {
        eorigin = query.edge;
        debug_gm << "Snap origin to closest edge " << query.point << std::endl;
      }
    }
  }
  {
    Neighbor_query query = closest_simplex(target);
    if (!is_ground_point(target) || query.dist < snapping_dist)
    {
      target = query.point;
      if (query.edge == Graph::null_edge())
      {
        vtarget = query.vertex;
        debug_gm << "Snap target to closest vertex " << query.point << std::endl;
      }
      else
      {
        etarget = query.edge;
        debug_gm << "Snap target to closest edge " << query.point << std::endl;
      }
    }
  }

  if (eorigin != Graph::null_edge() && eorigin == etarget) // moving along the same line
  {
    out.push_back (target);
    debug_gm << "Moving along line" << std::endl;
    return;
  }

  if (eorigin != Graph::null_edge())
  {
    vorigin = m_latest_graph.add_vertex({origin});
    GVertex v0 = m_latest_graph.source(eorigin);
    GVertex v1 = m_latest_graph.target(eorigin);
    m_latest_graph.delete_edge(eorigin);
    to_add.insert (std::make_pair (vorigin, v0));
    to_add.insert (std::make_pair (vorigin, v1));
  }
  else if (vorigin == Graph::null_vertex())
    vorigin = m_latest_graph.add_vertex({origin});

  if (etarget != Graph::null_edge())
  {
    vtarget = m_latest_graph.add_vertex({target});
    GVertex v0 = m_latest_graph.source(etarget);
    GVertex v1 = m_latest_graph.target(etarget);
    m_latest_graph.delete_edge(etarget);
    to_add.insert (std::make_pair (vtarget, v0));
    to_add.insert (std::make_pair (vtarget, v1));
  }
  else if (vtarget == Graph::null_vertex())
  {
    vtarget = m_latest_graph.add_vertex({target});
    debug_gm << "New target vertex " << vtarget << std::endl;
  }

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
    debug_gm << "No border intersected, going straight" << std::endl;

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
      debug_gm << "Trying to insert " << v << "-> " << n << std::endl;
      Point mid = midpoint (m_latest_graph[v].point, m_latest_graph[n].point);
      if (!is_ground_point(mid))
      {
        debug_gm << " -> mid point is not ground" << std::endl;
        continue;
      }
        
      Segment seg (m_latest_graph[v].point, m_latest_graph[n].point);
      if (intersects_border
          (m_latest_graph, seg,
           [&](const GEdge& e) -> bool
           {
             return ((e == eorigin || e == etarget) || m_latest_graph.edge_has_vertex(e, v));
           }))
      {
        debug_gm << " -> segment intersects border" << std::endl;
        continue;
      }

      to_add.insert (std::make_pair(v, n));
    }
  }

  for (const auto& p : to_add)
  {
    debug_gm << "New edge " << p.first << " -> " << p.second << std::endl;
    GEdge e = m_latest_graph.add_edge(p.first, p.second);
    m_latest_graph[e].border = false;
  }

  check (!m_latest_graph.incident_edges(vtarget).empty(), "Can't compute Djikstra from isolated vertex");

  m_latest_graph.clean();
  
  // Dijkstra
  m_latest_graph.validity();

  shortest_path(vorigin, vtarget, out);

  SOSAGE_TIMER_STOP(Ground_map__find_path);
}

void Ground_map::find_path (Point origin, Sosage::Vector direction, std::vector<Point>& out)
{
  SOSAGE_TIMER_START(Ground_map__find_path_gamepad);

  m_latest_graph = m_graph;

  GVertex vertex = Graph::null_vertex();
  GEdge edge = Graph::null_edge();
  Neighbor_query query = closest_simplex(origin);
  if (!is_ground_point(origin) || query.dist < snapping_dist)
  {
    origin = query.point;
    if (query.edge == Graph::null_edge())
      vertex = query.vertex;
    else
      edge = query.edge;
  }

  debug_gm << "Find path from " << origin << " with direction " << direction << std::endl;
  std::size_t repeat = 0;
  while (true)
  {
    // if free point
    //   edge = first intersected edge
    //   out.push_back(snapping point)
    // if edge point
    //   find next intersected edge
    //   if edge not reachable
    //     vertex = edge in the direction
    //     out.push_back(vertex)
    //   else
    //     edge = first intersected edge
    //     out.push_back(snapping point)
    // if vertex point
    //   find next intersected edge
    //   if edge not reachable
    //      if adjacent edge in direction
    //         vertex = next vertex in edge
    //      else
    //         break loop
    //   else
    //     edge = first intersected edge
    //     out.push_back(snapping point)

    debug_gm << "Iteration " << repeat << ": ";
    if (vertex != Graph::null_vertex())
    {
      debug_gm << "v" << vertex << " = " << m_latest_graph[vertex].point;
    }
    else if (edge != Graph::null_edge())
    {
      debug_gm << "e" << edge << "(v" << m_latest_graph.source(edge)
               << ", v" << m_latest_graph.target(edge)
               << ") = (" << m_latest_graph[m_latest_graph.source(edge)].point
               << ", " << m_latest_graph[m_latest_graph.target(edge)].point
               << ")" << std::endl;
    }

    if (!out.empty())
    {
      debug_gm << " latest=" << out.back() << std::endl;
    }

    ++ repeat;
    if (repeat > m_graph.num_edges() + m_graph.num_vertices())
    {
      debug_gm << "Loop has run for too long, first points are: ";
      for (std::size_t i = 0; i < 50; ++ i)
      {
        debug_gm << out[i] << " ";
      }
      debug_gm << std::endl;
      check(false, "Infinite loop while finding path");
    }

    if (vertex != Graph::null_vertex())
    {
      Neighbor_query query = closest_intersected_edge
                             (origin, direction,
                              [&](const GEdge& e) -> bool
                              {
                                return (m_latest_graph.edge_has_vertex(e, vertex)
                                        || m_latest_graph.edge_has_vertex(e, vertex));
                              });
      if (query)
      {
        vertex = query.vertex;
        edge = query.edge;
        origin = query.point;
        out.push_back(origin);
      }
      else
      {
        GVertex next_vertex = Graph::null_vertex();
        double scalar_max = 0.;
        for (GEdge e : m_latest_graph.incident_edges(vertex))
        {
          GVertex other = m_latest_graph.other(e, vertex);
          Sosage::Vector dir (origin, m_latest_graph[other].point);
          dir.normalize();

          double scalar = direction * dir;
          if (scalar > scalar_max)
          {
            scalar_max = scalar;
            next_vertex = other;
          }
        }

        if (next_vertex == Graph::null_vertex())
          break;
        vertex = next_vertex;
        origin = m_latest_graph[vertex].point;
        out.push_back(origin);
      }
    }
    else if (edge != Graph::null_edge())
    {
      Neighbor_query query = closest_intersected_edge
                             (origin, direction,
                              [&](const GEdge& e) -> bool
                              {
                                return e == edge;
                              });
      if (query)
      {
        vertex = query.vertex;
        edge = query.edge;
        origin = query.point;
        out.push_back(origin);
      }
      else
      {
        GVertex source = m_latest_graph.source(edge);
        GVertex target = m_latest_graph.target(edge);
        Sosage::Vector dir (m_latest_graph[source].point, m_latest_graph[target].point);
        double scalar = dir * direction;
        if (scalar == 0) // edge exactly perpendicular to direction, stop
          break;
        vertex = (scalar > 0 ? target : source);
        origin = m_latest_graph[vertex].point;
        out.push_back(origin);
      }
    }
    else // free point
    {
      Neighbor_query query = closest_intersected_edge
                             (origin, direction,
                              [&](const GEdge&) -> bool
                              {
                                return false;
                              });
      check(query, "Free point " + to_string(origin) + " with no intersected edge in direction " + to_string(direction));
      vertex = query.vertex;
      edge = query.edge;
      origin = query.point;
      out.push_back(origin);
    }

  }

  SOSAGE_TIMER_STOP(Ground_map__find_path_gamepad);
}

double Ground_map::z_at_point (const Point& p) const
{
  int x = p.X();
  int y = p.Y();
  if (x >= m_image->w) x = m_image->w - 1;
  if (x < 0) x = 0;
  if (y >= m_image->h) y = m_image->h - 1;
  if (y < 0) y = 0;

  std::array<unsigned char, 3> c = Core::Graphic::get_color (m_image, x, y);

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
      red = static_cast<unsigned char>((m_graph[m_graph.source(query.edge)].red * dtarget
                                       + m_graph[m_graph.target(query.edge)].red * dsource)
                                       / (dsource + dtarget));
    }
  }
  
  return m_back_z + (m_front_z - m_back_z) * (1. - (red / 255.));
}

bool Ground_map::is_ground_point (const Point& p) const
{
  debug_gm << "Is " << p << " a ground point?" << std::endl;
  int x = p.X();
  int y = p.Y();
  // Out of bound points can't be ground
  if (x >= m_image->w || y >= m_image->h || x < 0 || y < 0)
    return false;
  std::array<unsigned char, 3> c = Core::Graphic::get_color (m_image, x, y);
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
    debug_gm << "Dist(v" << v << "," << p << ") = " << dist << std::endl;
    if (dist < min_dist)
    {
      debug_gm << " -> v" << v << " is now best candidate" << std::endl;
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
      debug_gm << "Dist(e" << e << "," << p << ") = " << dist << std::endl;
      if (dist < min_dist)
      {
        debug_gm << " -> e" << e << " is now best candidate" << std::endl;
        point = proj;
        min_dist = dist;
        vertex = Graph::null_vertex();
        edge = e;
      }
    }
  }

  if (min_dist > snapping_dist)
  {
    debug_gm << "Free point because " << min_dist << " > " << snapping_dist << std::endl;
  }
  
  return {vertex, edge, min_dist, point};
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
    debug_gm << "Dist[" << v << "] = " << dist << std::endl;
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
        debug_gm << "Update Dist[" << next << "] = " << dist << std::endl;
        parent[std::size_t(next)] = current;
        debug_gm << "Update Parent[" << next << "] = " << current << std::endl;
        todo.push (std::make_pair (-dist, next));
      }
    }
  }
  
  std::vector<Point> path;
  while (vtarget != vorigin)
  {
    debug_gm << "Going back from " << vtarget << " to " << parent[vtarget] << std::endl;
    check (parent[vtarget] != Graph::null_vertex(), "Node has no parent");
    path.push_back (m_latest_graph[vtarget].point);
    vtarget = parent[std::size_t(vtarget)];
  }
  std::copy (path.rbegin(), path.rend(),
             std::back_inserter (out));
}

bool Ground_map::intersects_border (const Graph& g,
                                    const Segment& seg,
                                    const Edge_condition& condition) const
{
  for (GEdge e : g.edges())
  {
    if (!g.is_valid(e) || !g[e].border || condition(e))
      continue;
    Segment eseg (g[g.source(e)].point,
                  g[g.target(e)].point);
    if (intersect(seg, eseg))
      return true;
  }
  return false;
}

Ground_map::Neighbor_query Ground_map::closest_intersected_edge (const Point& p,
                                                                 const Sosage::Vector& direction,
                                                                 const Edge_condition& condition) const
{
  Segment seg (p, p + m_radius * direction);
  Neighbor_query out;

  const Graph& g = m_latest_graph;
  for (GEdge e : g.edges())
  {
    if (!g.is_valid(e) || !g[e].border || condition(e))
      continue;
    Segment eseg (g[g.source(e)].point,
                  g[g.target(e)].point);
    if (intersect(seg, eseg))
    {
      Point inter = intersection(seg, eseg);
      double dist = distance(p, inter);
      if (dist < out.dist)
      {
        out.edge = e;
        out.dist = dist;
        out.point = inter;
      }
    }
  }

  if (out.edge == Graph::null_edge())
    return Neighbor_query();

  for (GVertex v : { g.source(out.edge), g.target(out.edge) })
  {
    double dist = distance (out.point, g[v].point);
    if (dist < snapping_dist)
    {
      out.vertex = v;
      out.edge = Graph::null_edge();
      out.dist = dist;
      out.point = g[v].point;
    }
  }

  Point mid = midpoint(p, out.point);
  if (!is_ground_point(mid))
  {
    // Tricky case: midpoint might lie on edge or vertex
    // If it does, use it as next edge
    out = closest_simplex(mid);
    if ((out.edge != Graph::null_edge() && condition(out.edge))
        || out.dist > snapping_dist)
      return Neighbor_query();
  }

  return out;
}

} // namespace Sosage::Component
