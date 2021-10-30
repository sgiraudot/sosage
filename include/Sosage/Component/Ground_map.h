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

#include <Sosage/Component/Base.h>
#include <Sosage/Core/Graphic.h>
#include <Sosage/Utils/geometry.h>
#include <Sosage/Utils/graph.h>

#include <map>

namespace Sosage::Component
{

class Ground_map : public Base
{
  struct Vertex
  {
    Point point;
    unsigned char red = 0;
    double dist = 0;
  };

  struct Edge
  {
    bool border = true;
  };

  using Graph = Sosage::Graph<Vertex, Edge, false>;
  using GVertex = typename Graph::Vertex;
  using GEdge = typename Graph::Edge;
  using Edge_condition = std::function<bool(const GEdge&)>;

  double deviation(GVertex v) const;

  struct Neighbor_query
  {
    GVertex vertex = Graph::null_vertex();
    GEdge edge = Graph::null_edge();
    double dist = std::numeric_limits<double>::max();
    Point point;
    operator bool() const;
  };

  struct Compare_ordered_pair
  {
    template <typename Pair>
    Pair ordered (const Pair& p) const
    {
      if (p.first > p.second)
        return Pair(p.second, p.first);
      return p;
    }
    template <typename Pair>
    bool operator() (const Pair& a, const Pair& b) const
    {
      return ordered(a) < ordered(b);
    }
  };

  const double snapping_dist = 2.;
  Core::Graphic::Surface m_image;
  int m_radius;
  int m_front_z;
  int m_back_z;
  Graph m_graph;
  Graph m_latest_graph;

  GVertex add_vertex (std::map<Point, GVertex>& map_p2v,
                      const Point& p, const unsigned char& red);
  void build_graph (const std::function<void()>& callback);

public:

  Ground_map (const std::string& id, const std::string& file_name, int front_z, int back_z,
              const std::function<void()>& callback);
  void write (const std::string& filename);
  void read (const std::string& filename);

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
      functor (s, t, m_latest_graph[e].border);
    }
  }

  void find_path (Point origin, Point target, std::vector<Point>& out);
  void find_path (Point origin, Sosage::Vector direction, std::vector<Point>& out);
  double z_at_point (const Point& p) const;

  bool is_ground_point (const Point& p) const;
  Neighbor_query closest_simplex (const Point& p) const;

  void shortest_path (GVertex vorigin, GVertex vtarget,
                      std::vector<Point>& out);

  bool intersects_border (const Graph& g,
                          const Segment& seg,
                          const Edge_condition& condition) const;

  Neighbor_query closest_intersected_edge (const Point& p, const Sosage::Vector& direction,
                                           const Edge_condition& condition) const;

};

using Ground_map_handle = std::shared_ptr<Ground_map>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GROUND_MAP_H
