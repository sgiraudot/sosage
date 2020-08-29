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

Dialog::Dialog (const std::string& id)
  : Base(id)
{
  m_vin = m_graph.add_vertex();
  m_vout = m_graph.add_vertex();
}

Dialog::GVertex Dialog::add_vertex (const std::string& character, const std::string& line)
{
  GVertex out = m_graph.add_vertex (Vertex(character, line));
  std::cerr << "New vertex " << out << ": \"" << line << "\"" << std::endl;
  return out;
}

Dialog::GEdge Dialog::add_edge (Dialog::GVertex source, Dialog::GVertex target,
                                bool once, const std::string& line)
{
  std::cerr << "New edge from " << source << " to " << target << std::endl;
  return m_graph.add_edge (source, target, Edge (once, line));
}



} // namespace Sosage::Component::
