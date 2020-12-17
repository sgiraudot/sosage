/*
  [include/Sosage/Utils/graph.h]
  Adjacency-list based graph with templated vertices and edges.

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

#ifndef SOSAGE_UTILS_GRAPH_H
#define SOSAGE_UTILS_GRAPH_H

#include <Sosage/Config/config.h>
#include <Sosage/Utils/error.h>

#include <map>
#include <vector>

namespace Sosage
{

template <typename VertexType, typename EdgeType, bool Directed>
class Graph
{
public:

  template <typename T>
  struct Typed_int
  {
    int value;
    
    Typed_int (const Typed_int& ti) : value (static_cast<int>(ti)) { }
    Typed_int (const std::size_t& value) : value (static_cast<int>(value)) { }
    Typed_int () : value (static_cast<int>(-1)) { }
    Typed_int operator= (const Typed_int& ti) { value = ti.value; return *this; }
    operator std::size_t() const { return static_cast<std::size_t>(value); }
    bool operator== (const Typed_int& ti) const { return value == ti.value; }
    bool operator!= (const Typed_int& ti) const { return value != ti.value; }
    bool operator<  (const Typed_int& ti) const { return value < ti.value; }
    Typed_int& operator++ () { ++ value; return *this; }
    Typed_int& operator-- () { -- value; return *this; }
    Typed_int operator++ (int) { Typed_int tmp(*this); ++ value; return tmp; }
    Typed_int operator-- (int) { Typed_int tmp(*this); -- value; return tmp; }

  };

  using Vertex = Typed_int<VertexType>;
  using Edge = Typed_int<EdgeType>;

  struct GVertex
  {
    VertexType base;
    std::vector<Edge> incident_edges;

    GVertex (const VertexType& base) : base (base) { }
  };

  struct GEdge
  {
    EdgeType base;
    Vertex source;
    Vertex target;

    GEdge (const EdgeType& base, Vertex source, Vertex target)
      : base (base), source (source), target (target)
    {
      if (!Directed && source > target)
        std::swap (this->source, this->target);
    }

    Vertex other (const Vertex& v) const
    {
      if (v == source)
        return target;
      check (v == target, "Vertex is not in edge");
      return source;
    }
  };
  
  template <typename GSimplex, typename Simplex>
  class Container
  {
    std::vector<GSimplex> m_content;

    struct iterator
    {
      Simplex value;

      Simplex operator*() { return value; }
      iterator& operator++() { value++; return *this; }

      bool operator!= (const iterator& other) const { return value != other.value; }

      iterator (const Simplex& value = -1) : value(value) { }
    };

  public:

    Simplex push_back (const GSimplex& base)
    {
      Simplex out = Simplex(m_content.size());
      m_content.push_back (GSimplex(base));
      return out;
    }

    GSimplex& operator[] (const Simplex& s)
    {
      dbg_check (std::size_t(s) < m_content.size(), "Trying to access Simplex " + std::to_string(s) + "/" + std::to_string(m_content.size()));
      return m_content[std::size_t(s)];
    }
    const GSimplex& operator[] (const Simplex& s) const
    {
      dbg_check (std::size_t(s) < m_content.size(), "Trying to access Simplex " + std::to_string(s) + "/" + std::to_string(m_content.size()));
      return m_content[std::size_t(s)];
    }

    std::size_t size() const { return m_content.size(); }

    iterator begin() const { return iterator(0); }
    iterator end() const { return iterator(Simplex(m_content.size())); }

    void swap (Container& c)
    {
      m_content.swap(c.m_content);
    }
  };

  using Vertices = Container<GVertex, Vertex>;
  using Edges = Container<GEdge, Edge>;
  
  Vertices m_vertices;
  Edges m_edges;

public:

  static Vertex null_vertex() { return Vertex(-1); }
  static Edge null_edge() { return Edge(-1); }

  VertexType& operator[] (const Vertex& v) { return m_vertices[v].base; }
  const VertexType& operator[] (const Vertex& v) const { return m_vertices[v].base; }
  EdgeType& operator[] (const Edge& e) { return m_edges[e].base; }
  const EdgeType& operator[] (const Edge& e) const { return m_edges[e].base; }

  Vertices& vertices() { return m_vertices; }
  const Vertices& vertices() const { return m_vertices; }
  Edges& edges() { return m_edges; }
  const Edges& edges() const { return m_edges; }

  std::size_t num_vertices() const { return m_vertices.size(); }
  std::size_t num_edges() const { return m_edges.size(); }

  std::vector<Edge>& incident_edges (const Vertex& v) { return m_vertices[v].incident_edges; }
  const std::vector<Edge>& incident_edges (const Vertex& v) const { return m_vertices[v].incident_edges; }

  bool is_valid (const Edge& e) const { return m_edges[e].source != null_vertex() && m_edges[e].target != null_vertex(); }

  Vertex source (const Edge& e) const { return m_edges[e].source; }
  Vertex target (const Edge& e) const { return m_edges[e].target; }

  Edge incident_edge (const Vertex& v, std::size_t idx) const { return m_vertices[v].incident_edges[idx]; }
  Vertex incident_vertex (const Vertex& v, std::size_t idx) const
  {
    return m_edges[m_vertices[v].incident_edges[idx]].other(v);
  }

  bool edge_has_vertex (const Edge& e, const Vertex& v) const
  {
    return (m_edges[e].source == v ||
            m_edges[e].target == v);
  }

  Vertex add_vertex (const VertexType& base = VertexType())
  {
    return m_vertices.push_back (GVertex(base));
  }

  Edge add_edge (const Vertex& va, const Vertex& vb, const EdgeType& base = EdgeType())
  {
    Edge out = m_edges.push_back (GEdge(base, va, vb));
    m_vertices[va].incident_edges.push_back(out);
    if (!Directed)
      m_vertices[vb].incident_edges.push_back(out);
    return out;
  }

  bool is_edge (const Vertex& va, const Vertex& vb) const
  {
    if (Directed || incident_edges(va).size() < incident_edges(vb).size())
    {
      for (std::size_t i = 0; i < incident_edges(va).size(); ++ i)
        if (incident_vertex(va, 0) == vb)
          return true;
    }
    else
    {
      for (std::size_t i = 0; i < incident_edges(vb).size(); ++ i)
        if (incident_vertex(vb, 0) == va)
          return true;
    }
    return false;
  }

  void delete_edge (const Edge& e)
  {
    Vertex vsource = source(e);
    Vertex vtarget = target(e);

    m_edges[e].source = null_vertex();
    m_edges[e].target = null_vertex();

    if (Directed)
    {
      for (auto it = m_vertices[vsource].incident_edges.begin();
           it != m_vertices[vsource].incident_edges.end(); ++ it)
        if (*it == e)
        {
          m_vertices[vsource].incident_edges.erase(it);
          break;
        }
    }
    else
    {
      for (Vertex v : { vsource, vtarget })
        for (auto it = m_vertices[v].incident_edges.begin();
             it != m_vertices[v].incident_edges.end(); ++ it)
          if (*it == e)
          {
            m_vertices[v].incident_edges.erase(it);
            break;
          }
    }
  }

  void clean()
  {
    Vertices new_vertices;
    std::map<Vertex, Vertex> map_v2v;

    for (std::size_t i = 0; i < m_vertices.size(); ++ i)
      if (!m_vertices[i].incident_edges.empty())
      {
        map_v2v.insert (std::make_pair (Vertex(i), Vertex(new_vertices.size())));
        Vertex new_vertex = new_vertices.push_back (m_vertices[i]);
        new_vertices[new_vertex].incident_edges.clear();
      }
    
    Edges new_edges;
    for (std::size_t i = 0; i < m_edges.size(); ++ i)
      if (source(i) != null_vertex() && target(i) != null_vertex())
      {
        Vertex new_source = map_v2v[source(i)];
        Vertex new_target = map_v2v[target(i)];
        Edge new_edge = new_edges.push_back (GEdge(m_edges[i].base, new_source, new_target));
        new_vertices[new_source].incident_edges.push_back(new_edge);
        if (!Directed)
          new_vertices[new_target].incident_edges.push_back(new_edge);
      }

    m_vertices.swap (new_vertices);
    m_edges.swap (new_edges);
  }

  void validity()
  {
    debug("checking graph");
    for (Vertex v : vertices())
    {
      for (std::size_t i = 0; i < incident_edges(v).size(); ++ i)
      {
        Edge e = incident_edge(v, i);
        if (Directed)
        {
          check (source(e), "Ill-formed edge");
        }
        else
        {
          check (source(e) == v || target(e) == v, "Ill-formed edge");
        }
      }
    }
  }

  
};

} // namespace Sosage

#endif // SOSAGE_UTILS_GRAPH_H
