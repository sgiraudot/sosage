/*
  [include/Sosage/Utils/geometry.h]
  Basic geometric objects and functions (points, distances, etc.).

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

#ifndef SOSAGE_UTILS_GEOMETRY_H
#define SOSAGE_UTILS_GEOMETRY_H

#include <Sosage/Config/config.h>
#include <Sosage/Utils/error.h>

#include <cmath>
#include <iostream>

namespace Sosage
{

template <typename T>
inline T square (const T& t) { return t*t; }

inline double distance (double xa, double ya, double xb, double yb)
{
  return std::sqrt (square(xa - xb) + square(ya - yb));
}

enum Orientation
{
  COLINEAR,
  CLOCKWISE,
  COUNTERCLOCKWISE
};

struct Box
{
  double xmin;
  double ymin;
  double xmax;
  double ymax;

  Box (double xmin, double ymin, double xmax, double ymax)
    : xmin(xmin), ymin(ymin), xmax(xmax), ymax(ymax)
  { }

  friend Box operator+ (const Box& a, const Box& b)
  {
    return Box (std::min (a.xmin, b.xmin),
                std::min (a.xmin, b.xmin),
                std::max (a.xmax, b.xmax),
                std::max (a.ymax, b.ymax));
  }

  friend bool intersect (const Box& a, const Box& b)
  {
    if (a.xmax < b.xmin || b.xmax < a.xmin)
      return false;
    if (a.ymax < b.ymin || b.ymax < a.ymin)
      return false;
    return true;
  }
};

class Vector;

class Point
{
  double m_x;
  double m_y;
  
public:

  Point (const double& x = 0, const double& y = 0)
    : m_x (x), m_y (y)
  { }
  
  Point (const std::pair<double, double>& coord)
    : Point (coord.first, coord.second)
  { }

  double x() const { return m_x; }
  double y() const { return m_y; }
  int X() const { return std::lround(m_x); }
  int Y() const { return std::lround(m_y); }

  Box box() const { return Box(m_x, m_y, m_x, m_y); }

  friend std::string to_string(const Point& p)
  {
    return "Point(" + std::to_string(p.m_x) + ";" + std::to_string(p.m_y) + ")";
  }

  friend std::ostream& operator<< (std::ostream& os, const Point& c)
  {
    os << "Point(" << c.m_x << ";" << c.m_y << ")";
    return os;
  }

  friend Point operator+ (const Point& a, const Point& b)
  {
    return Point (a.m_x + b.m_x, a.m_y + b.m_y);
  }
  friend Point operator* (const double& a, const Point& b)
  {
    return Point (a * b.m_x, a * b.m_y);
  }
  friend bool operator== (const Point& a, const Point& b)
  {
    return ((a.m_x == b.m_x) && (a.m_y == b.m_y));
  }

  friend bool operator< (const Point& a, const Point& b)
  {
    if (a.m_x == b.m_x)
      return (a.m_y < b.m_y);
    return a.m_x < b.m_x;
  }
  
  friend double distance (const Point& a, const Point& b)
  {
    return distance (a.x(), a.y(), b.x(), b.y());
  }

  friend Point midpoint (const Point& a, const Point& b)
  {
    return Point ((a.x() + b.x()) / 2, (a.y() + b.y()) / 2);
  }

  friend Orientation orientation (const Point& a, const Point& b, const Point& c)
  {
    int val = (b.m_y - a.m_y) * (c.m_x - b.m_x) -
              (b.m_x - a.m_x) * (c.m_y - b.m_y); 

    if (val == 0)
      return COLINEAR;
    if (val > 0)
      return CLOCKWISE;
    // else
    return COUNTERCLOCKWISE;
  }
};


class Vector : public Point
{
public:

  Vector (double x, double y)
    : Point (x, y)
  { }

  Vector (const Point& a, const Point& b)
    : Point (b.x() - a.x(), b.y() - a.y())
  {
    
  }
  
  Vector (const Point& p)
    : Point (p.x(), p.y())
  {
    
  }
  
  double length() const
  {
    return std::sqrt(square (x()) + square (y()));
  }
  void normalize()
  {
    double l = length();
    *this = Vector(x() / l, y() / l);
  }

  friend Point operator+ (const Point& a, const Vector& b)
  {
    return Point(a.x() + b.x(), a.y() + b.y());
  }
  friend Point operator- (const Point& a, const Vector& b)
  {
    return Point(a.x() - b.x(), a.y() - b.y());
  }
  friend double operator* (const Vector& a, const Vector& b)
  {
    return a.x() * b.x() + a.y() * b.y();
  }
  friend Vector operator* (const double& a, const Vector& b)
  {
    return Vector (a * b.x(), a * b.y());
  }
};

class Line
{
  Point m_reference;
  Vector m_direction;
  
public:

  Line (const Point& a, const Point& b)
    : m_reference (a),
      m_direction (a, b)
  {
    m_direction.normalize();
  }

  Point projection (const Point& p)
  {
    Vector ref (m_reference, p);

    double length = ref * m_direction;

    return m_reference + length * m_direction;
  }

  const Vector& direction() const { return m_direction; }

};

class Segment
{
  Point m_source;
  Point m_target;

public:

  Segment (const Point& source, const Point& target)
    : m_source (source), m_target (target)
  { }

  Vector to_vector() const
  {
    return Vector(m_source, m_target);
  }

  Line to_line() const
  {
    return Line(m_source, m_target);
  }

  Box box() const { return m_source.box() + m_target.box(); }

  double length() const
  {
    return distance (m_source.x(), m_source.y(), m_target.x(), m_target.y());
  }
  
  std::pair<Point, bool> projection (const Point& p) const
  {
    Line l = to_line();
    Point proj = l.projection(p);
    Vector vproj (m_source, proj);
    return std::make_pair (proj,
                           (vproj * l.direction() > 0
                            && distance (m_source, proj) < length()));
  }
  
  friend std::ostream& operator<< (std::ostream& os, const Segment& s)
  {
    os << "Segment(" << s.m_source.x() << ";" << s.m_source.y()
       << " -> " << s.m_target.x() << ";" << s.m_target.y() << ")";
    return os;
  }


  friend bool intersect (const Segment& a, const Segment& b)
  {
    // Quick test with boxes
    // if (!intersect (a.box(), b.box()))
    //   return false;

    Orientation o0 = orientation(a.m_source, a.m_target, b.m_source);
    Orientation o1 = orientation(a.m_source, a.m_target, b.m_target);
    Orientation o2 = orientation(b.m_source, b.m_target, a.m_source);
    Orientation o3 = orientation(b.m_source, b.m_target, a.m_target);
    
    if (o0 != o1 && o2 != o3)
      return true;
    if (o0 == COLINEAR && a.projection(b.m_source).second)
      return true; 
    if (o1 == COLINEAR && a.projection(b.m_target).second)
      return true; 
    if (o2 == COLINEAR && b.projection(a.m_source).second)
      return true;
    if (o3 == COLINEAR && b.projection(a.m_target).second)
      return true;

    return false;
  }
};

} // namespace Sosage

#endif // SOSAGE_UTILS_GEOMETRY_H
