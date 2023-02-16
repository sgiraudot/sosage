/*
  [src/Sosage/Component/Dialog.cpp]
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

#include <Sosage/Component/Dialog.h>

namespace Sosage::Component
{


Dialog::Dialog (const std::string& entity, const std::string& component, const std::string& end)
  : Base(entity, component)
{
  m_vin = m_graph.add_vertex({"", "IN", ""});
  m_vout = m_graph.add_vertex({end,"OUT", ""});
}

Dialog::GVertex Dialog::add_vertex (const std::string& character,
                                    const std::string& line,
                                    const std::string& signal)
{
  GVertex out = m_graph.add_vertex ({character, line, signal});
  return out;
}

Dialog::GEdge Dialog::add_edge (GVertex source, GVertex target, const std::string& line,
                                const std::string& condition, bool unless, bool displayed)
{
  return m_graph.add_edge (source, target, {line, condition, unless, displayed, true});
}

bool Dialog::has_incident_edges (Dialog::GVertex v)
{
  return !m_graph.incident_edges(v).empty();
}

Dialog::GVertex Dialog::current() const
{
  return m_current;
}

Dialog::GVertex Dialog::vertex_in() const
{
  return m_vin;
}

Dialog::GVertex Dialog::vertex_out() const
{
  return m_vout;
}

void Dialog::init (Dialog::GVertex current)
{
  if (current == Graph::null_vertex())
  {
    m_current = m_vin;
    next();
  }
  else
    m_current = current;
}

void Dialog::next()
{
  m_current = m_graph.incident_vertex(m_current, 0);
}

std::string Dialog::next (int choice)
{
  std::string out = "";
  int i = 0;
  for (GEdge e : m_graph.incident_edges(m_current))
    if (m_graph[e].enabled)
    {
      if (i == choice)
      {
        if (m_graph[e].condition == "said")
          out = entity() + std::to_string(std::size_t(e));
        m_current = m_graph.target(e);
        return out;
      }
      ++ i;
    }
  return out;
}

bool Dialog::is_displayed (int choice)
{
  int i = 0;
  for (GEdge e : m_graph.incident_edges(m_current))
    if (m_graph[e].enabled)
    {
      if (i == choice)
      {
        return m_graph[e].displayed;
      }
      ++ i;
    }
  return true;
}

bool Dialog::is_over() const
{
  return (m_current == m_vout);
}

bool Dialog::is_line() const
{
  return (m_graph[m_current].character != "");
}

bool Dialog::has_signal() const
{
  return (m_graph[m_current].signal != "");
}

std::pair<std::string, std::string> Dialog::line() const
{
  return std::make_pair (m_graph[m_current].character, m_graph[m_current].line);
}

const std::string& Dialog::signal() const
{
  return m_graph[m_current].signal;
}

} // namespace Sosage::Component
