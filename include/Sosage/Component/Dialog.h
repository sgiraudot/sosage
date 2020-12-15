/*
  [include/Sosage/Component/Dialog.h]
  A dialog stored as a graph.

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

#ifndef SOSAGE_COMPONENT_DIALOG_H
#define SOSAGE_COMPONENT_DIALOG_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/graph.h>

namespace Sosage::Component
{

class Dialog : public Base
{
  struct Vertex
  {
    std::string character;
    std::string line;

    Vertex (const std::string& character = "", const std::string& line = "")
      : character (character), line (line) { }
  };

  enum Edge_status { ALWAYS, ONCE, DISABLED };

  struct Edge
  {
    Edge_status status;
    std::string line;

    Edge (bool once = false, const std::string& line = "")
      : status(once ? ONCE : ALWAYS), line(line) { }
  };

  using Graph = Sosage::Graph<Vertex, Edge, true>;

public:

  using GVertex = typename Graph::Vertex;
  using GEdge = typename Graph::Edge;

private:

  Graph m_graph;
  GVertex m_vin;
  GVertex m_vout;
  GVertex m_current;

public:

  Dialog (const std::string& id, const std::string& end = "");

  GVertex add_vertex (const std::string& character = "",
                      const std::string& line = "");

  GEdge add_edge (GVertex source, GVertex target,
                  bool once = false, const std::string& line = "");

  bool has_incident_edges (GVertex v) { return !m_graph.incident_edges(v).empty(); }

  GVertex current() const { return m_current; }
  GVertex vertex_in() const { return m_vin; }
  GVertex vertex_out() const { return m_vout; }

  void init(GVertex current = Graph::null_vertex())
  {
    if (current == Graph::null_vertex())
    {
      m_current = m_vin;
      next();
    }
    else
      m_current = current;
  }

  void next()
  {
    m_current = m_graph.incident_vertex(m_current, 0);
  }

  void next (int choice)
  {
    int i = 0;
    for (GEdge e : m_graph.incident_edges(m_current))
      if (m_graph[e].status != DISABLED)
      {
        if (i == choice)
        {
          if (m_graph[e].status == ONCE)
            m_graph[e].status = DISABLED;
          m_current = m_graph.target(e);
          return;
        }
        ++ i;
      }
  }

  bool is_over() const { return (m_current == m_vout); }

  bool is_line() const
  {
    return (m_graph[m_current].character != "");
  }

  std::pair<std::string, std::string> line() const
  { return std::make_pair (m_graph[m_current].character, m_graph[m_current].line); }

  template <typename Container>
  void get_choices (Container& choices)
  {
    for (GEdge e : m_graph.incident_edges(m_current))
      if (m_graph[e].status != DISABLED)
        choices.push_back (m_graph[e].line);
  }

};


} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_DIALOG_H
