/*
  [include/Sosage/Component/Menu.h]
  Menu and information boxes, windows, etc.

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

#ifndef SOSAGE_COMPONENT_MENU_H
#define SOSAGE_COMPONENT_MENU_H

#include <Sosage/Component/Base.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Utils/enum.h>
#include <Sosage/Utils/graph.h>

namespace Sosage::Component
{

class Menu : public Base
{

  struct Vertex
  {
    Split_direction direction;
    std::vector<Image_handle> image;
    Position_handle position;
    std::size_t current = 0;
  };

  struct Edge {};

  using Tree = Graph<Vertex, Edge, true>;
  using GVertex = Tree::Vertex;
  using GEdge = Tree::Edge;

  class Vertex_wrapper
  {
  private:
    Tree& tree;
    GVertex vertex;
  public:
    Vertex_wrapper (Tree& tree, GVertex vertex);
    Split_direction direction() const;
    bool has_image();
    Image_handle image() const;
    Position_handle position() const;
    void init (Image_handle image, Position_handle position);
    void add (Image_handle image);
    std::string change_setting (const std::string& setting, int diff);
    Vertex_wrapper operator[] (const std::size_t idx) const;
    void split (Split_direction direction, std::size_t nb_children);
    std::size_t nb_children() const;
    void hide();
    void apply (const std::function<void(Image_handle)>& func);
    void update_setting (const std::string& setting, const std::string& value);
  };

  Tree m_tree;
  GVertex m_root;

public:

  using Node = Vertex_wrapper;

  Menu (const std::string& id);
  Node operator[] (const std::size_t idx);
  void split (Split_direction direction, std::size_t nb_children);
  std::size_t nb_children() const;
  void hide();
  void apply (const std::function<void(Image_handle)>& func);
  void update_setting (const std::string& setting, const std::string& value);
  std::string increment (const std::string& setting);
  std::string decrement (const std::string& setting);
  Node root() const;
};

using Menu_handle = std::shared_ptr<Menu>;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_MENU_H
