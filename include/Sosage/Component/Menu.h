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

#include <Sosage/Component/Handle.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Utils/enum.h>
#include <Sosage/Utils/graph.h>

#include <stack>

namespace Sosage
{

namespace Component
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
    Vertex_wrapper (Tree& tree, GVertex vertex)
      : tree(tree), vertex(vertex) {}

    Split_direction direction() const { return tree[vertex].direction; }

    Image_handle image() const { return tree[vertex].image[tree[vertex].current]; }
    Position_handle position() const { return tree[vertex].position; }

    void init (Image_handle image, Position_handle position)
    {
      tree[vertex].image.emplace_back(image);
      tree[vertex].position = position;
    }

    void add (Image_handle image)
    {
      tree[vertex].image.emplace_back(image);
    }

    std::string change_setting (const std::string& setting, int diff)
    {
      if (nb_children() == 0)
      {
        if (tree[vertex].image.size() > 1 &&
            position()->entity() == setting)
        {
          image()->on() = false;
          if (diff == 1)
          {
            ++ tree[vertex].current;
            if (tree[vertex].current == tree[vertex].image.size())
              tree[vertex].current = 0;
          }
          else
          {
            if (tree[vertex].current == 0)
              tree[vertex].current = tree[vertex].image.size();
            -- tree[vertex].current;
          }

          image()->on() = true;
          std::string entity = image()->entity();
          return std::string (entity.begin() + setting.size() + 1, entity.end());
        }
      }
      else
        for (std::size_t i = 0; i < nb_children(); ++ i)
        {
          std::string out = (*this)[i].change_setting (setting, diff);
          if (out != "")
            return out;
        }
      return "";
    }

    Vertex_wrapper operator[] (const std::size_t idx) const
    {
      return Vertex_wrapper (tree, tree.incident_vertex(vertex, idx));
    }

    void split (Split_direction direction, std::size_t nb_children)
    {
      check(tree[vertex].direction == NO_SPLIT, "Trying to split already splitted vertex");
      tree[vertex].direction = direction;
      for (std::size_t i = 0; i < nb_children; ++ i)
      {
        GVertex v = tree.add_vertex();
        tree.add_edge(vertex, v);
      }
    }

    std::size_t nb_children() const { return tree.incident_edges(vertex).size(); }

    Sosage::Vector size() const
    {
      if (nb_children() == 0)
      {
        int x = 0, y = 0;
        for (Image_handle img : tree[vertex].image)
        {
          double scale = 1.;
          if (img->scale() < 1.)
            scale = img->scale() / 0.75;
          x = std::max(x, int(img->width() * scale));
          y = std::max(y, int(img->height() * scale));
        }
        return Sosage::Vector(x + 10, y + 10);
      }
      else if (nb_children() == 1)
      {
        Sosage::Vector s = (*this)[0].size();
        return s + Sosage::Vector(10, 10);
      }
      // else
      double x = 0, y = 0;
      for (std::size_t i = 0; i < nb_children(); ++ i)
      {
        Sosage::Vector s = (*this)[i].size();
        if (direction() == HORIZONTALLY)
        {
          x += s.x();
          y = std::max(y, s.y());
        }
        else
        {
          check (direction() == VERTICALLY, "Non-empty node with no split direction");
          x = std::max(x, s.x());
          y += s.y();
        }
      }
      return Sosage::Vector(x, y);
    }

    void set_position (double x, double y)
    {
      if (nb_children() == 0)
      {
        image()->on() = true;
        position()->set(Point(x,y));
      }
      else if (nb_children() == 1)
      {
        image()->on() = true;
        position()->set(Point(x,y));
        (*this)[0].set_position(x,y);
      }
      else
      {
        Sosage::Vector s = size();
        double dx = (direction() == HORIZONTALLY ? x - s.x() / 2 : x);
        double dy = (direction() == VERTICALLY ? y - s.y() / 2 : y);
        for (std::size_t i = 0; i < nb_children(); ++ i)
        {
          Sosage::Vector s = (*this)[i].size();

          if (direction() == HORIZONTALLY)
            dx += s.x() / 2;
          else
            dy += s.y() / 2;

          (*this)[i].set_position (dx, dy);

          if (direction() == HORIZONTALLY)
            dx += s.x() / 2;
          else
            dy += s.y() / 2;

        }
      }
    }

    void hide()
    {
      if (nb_children() == 0)
        image()->on() = false;
      else if (nb_children() == 1)
      {
        image()->on() = false;
        (*this)[0].hide();
      }
      else
        for (std::size_t i = 0; i < nb_children(); ++ i)
          (*this)[i].hide();
    }

    void apply (const std::function<void(Image_handle)>& func)
    {
      if (nb_children() < 2)
        func(image());
      for (std::size_t i = 0; i < nb_children(); ++ i)
        (*this)[i].apply(func);
    }

    void update_setting (const std::string& setting, const std::string& value)
    {
      if (nb_children() == 0)
      {
        if (tree[vertex].image.size() > 1 &&
            position()->entity() == setting)
        {
          for (std::size_t i = 0; i < tree[vertex].image.size(); ++ i)
          {
            std::string entity = tree[vertex].image[i]->entity();
            std::string v (entity.begin() + setting.size() + 1, entity.end());
            if (v == value)
            {
              tree[vertex].current = i;
              return;
            }
          }
          check (false, "Value " + value + " not found in setting " + setting);
        }
      }
      else
        for (std::size_t i = 0; i < nb_children(); ++ i)
          (*this)[i].update_setting (setting, value);
    }
  };


  Tree m_tree;
  GVertex m_root;

public:

  using Node = Vertex_wrapper;

  Menu (const std::string& id)
    : Base(id)
  {
    m_root = m_tree.add_vertex();
  }

  Node operator[] (const std::size_t idx)
  {
    return root()[idx];
  }

  void split (Split_direction direction, std::size_t nb_children)
  {
    root().split(direction, nb_children);
  }

  std::size_t nb_children() const { return root().nb_children(); }

  Sosage::Vector size() const { return root().size(); }

  void set_position (double x, double y) { root().set_position(x, y); }

  void hide() { root().hide(); }

  void apply (const std::function<void(Image_handle)>& func) { root().apply(func); }

  void update_setting (const std::string& setting, const std::string& value)
  {
    root().update_setting (setting, value);
  }

  std::string increment (const std::string& setting)
  {
    return root().change_setting (setting, 1);
  }
  std::string decrement (const std::string& setting)
  {
    return root().change_setting (setting, -1);
  }

  Node root() const { return Node(const_cast<Tree&>(m_tree), m_root); }

};

using Menu_handle = std::shared_ptr<Menu>;

} // namespace Component

} // namespace Sosage

#endif // SOSAGE_COMPONENT_MENU_H
