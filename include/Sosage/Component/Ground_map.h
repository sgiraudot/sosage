/*
  [include/Sosage/Component/Ground_map.h]
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

#ifndef SOSAGE_COMPONENT_GROUND_MAP_H
#define SOSAGE_COMPONENT_GROUND_MAP_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/geometry.h>
#include <Sosage/Utils/graph.h>

#include <queue>
#include <map>

namespace Sosage::Component
{

class Ground_map : public Base
{
  struct Vertex
  {
    Point point;
    unsigned char red;
    double dist;
    Vertex (const Point& point = Point(), unsigned char red = 0)
      : point (point), red (red), dist(0)
    { }
  };

  struct Edge { };

  typedef Sosage::Graph<Vertex, Edge> Graph;
  typedef typename Graph::Vertex GVertex;
  typedef typename Graph::Edge GEdge;

  double deviation(GVertex v) const
  {
    if (m_graph.incident_edges(v).size() != 2)
      return std::numeric_limits<double>::max();

    GVertex v0 = m_graph.incident_vertex(v, 0);
    GVertex v1 = m_graph.incident_vertex(v, 1);

    Line l (m_graph[v0].point, m_graph[v1].point);
    Point proj = l.projection (m_graph[v].point);
    return distance (m_graph[v].point, proj);
  }

  struct Neighbor_query
  {
    const GVertex vertex;
    const GEdge edge;
    double dist;
    Point point;

    Neighbor_query (const GVertex& vertex, const GEdge& edge,
                    double dist, const Point& point)
      : vertex (vertex), edge (edge), dist (dist), point (point)
    { }
  };

  struct Compare_ordered_pair
  {
    template <typename Pair>
    Pair ordered (const Pair& p)
    {
      if (p.first > p.second)
        return Pair(p.second, p.first);
      return p;
    }
    template <typename Pair>
    bool operator() (const Pair& a, const Pair& b)
    {
      return ordered(a) < ordered(b);
    }
  };

  Core::Graphic::Surface m_image;
  int m_front_z;
  int m_back_z;
  Graph m_graph;
  Graph m_latest_graph;
  
  GVertex add_vertex (std::map<Point, GVertex>& map_p2v,
                      const Point& p, const unsigned char& red)
  {
    auto it = map_p2v.insert (std::make_pair (p, Graph::null_vertex()));
    if (it.second)
    {
      GVertex v = m_graph.add_vertex (Vertex(p, red));
      it.first->second = v;
    }
    return it.first->second;
  }

public:

  Ground_map (const std::string& id, const std::string& file_name, int front_z, int back_z);

  ~Ground_map ();

  template <typename Functor>
  void for_each_vertex (const Functor& functor) const
  {
    for (GVertex v : m_latest_graph.vertices())
      functor (m_latest_graph[v].point);
  }

  template <typename Functor>
  void for_each_edge (const Functor& functor) const
  {
    for (GEdge e : m_latest_graph.edges())
    {
      const Point& s = m_latest_graph[m_latest_graph.source(e)].point;
      const Point& t = m_latest_graph[m_latest_graph.target(e)].point;
      functor (s, t);
    }
  }
  
  void find_path (Point origin, Point target,
                  std::vector<Point>& out);
  double z_at_point (const Point& p) const;

  bool is_ground_point (const Point& p) const;
  Neighbor_query closest_simplex (const Point& p) const;

  void shortest_path (GVertex vorigin, GVertex vtarget,
                      std::vector<Point>& out);

};
typedef std::shared_ptr<Ground_map> Ground_map_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GROUND_MAP_H
