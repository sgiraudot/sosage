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

#include <Sosage/Component/Base.h>
#include <Sosage/Utils/graph.h>

namespace Sosage::Component
{

class Dialog : public Base
{
  struct Vertex
  {
    std::string character;
    std::string line;
    std::string signal;
  };

  struct Edge
  {
    std::string line;
    std::string condition;
    bool unless;
    bool displayed;
    bool enabled;
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

  Dialog (const std::string& entity, const std::string& component, const std::string& end = "");
  GVertex add_vertex (const std::string& character = "",
                      const std::string& line = "",
                      const std::string& signal = "");
  GEdge add_edge (GVertex source, GVertex target, const std::string& line = "",
                  const std::string& condition = "", bool unless = false, bool displayed = true);
  bool has_incident_edges (GVertex v);
  GVertex current() const;
  GVertex vertex_in() const;
  GVertex vertex_out() const;
  void init(GVertex current = Graph::null_vertex());
  void next();
  std::string next (int choice);
  bool is_displayed (int choice);
  bool is_over() const;
  bool is_line() const;
  bool has_signal() const;
  std::pair<std::string, std::string> line() const;
  const std::string& signal() const;

  template <typename Container, typename Content>
  void get_choices (Container& choices, Content& content)
  {
    for (GEdge e : m_graph.incident_edges(m_current))
    {
      if (m_graph[e].condition != "")
      {
        std::string condition = (m_graph[e].condition == "said"
                                 ? entity() + std::to_string(std::size_t(e)) : m_graph[e].condition);
        m_graph[e].enabled = (content.signal(condition, "signal") != m_graph[e].unless);
      }

      if (!m_graph[e].enabled)
        continue;

      choices.push_back (m_graph[e].line);
    }
  }

  STR_NAME("Dialog");
};

using Dialog_handle = std::shared_ptr<Dialog>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_DIALOG_H
