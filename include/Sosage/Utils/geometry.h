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

#include <Sosage/Utils/enum.h>
#include <Sosage/Utils/error.h>

#include <cmath>
#include <iostream>

namespace Sosage
{

int round (const double& x);

template <typename T>
inline T square (const T& t) { return t*t; }

double distance (double xa, double ya, double xb, double yb);
double angle (double x, double y);

struct Box
{
  double xmin;
  double ymin;
  double xmax;
  double ymax;

  friend Box operator+ (const Box& a, const Box& b)
  {
    return {std::min (a.xmin, b.xmin),
          std::min (a.xmin, b.xmin),
          std::max (a.xmax, b.xmax),
          std::max (a.ymax, b.ymax)};
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

  Point (const double& x = 0, const double& y = 0);
  Point (const std::pair<double, double>& coord);
  double x() const;
  double y() const;
  int X() const;
  int Y() const;
  Box box() const;
  bool is_invalid() const;
  static Point invalid();
  static Point right();
  static Point left();

  friend std::string to_string(const Point& p)
  {
    return "Point(" + std::to_string(p.m_x) + ";" + std::to_string(p.m_y) + ")";
  }

  friend std::ostream& operator<< (std::ostream& os, const Point& c)
  {
    os.precision(18);
    os << "Point(" << c.m_x << ";" << c.m_y << ")";
    return os;
  }

  friend Point operator+ (const Point& a, const Point& b)
  {
    return Point (a.m_x + b.m_x, a.m_y + b.m_y);
  }
  friend Point operator- (const Point& a, const Point& b)
  {
    return Point (a.m_x - b.m_x, a.m_y - b.m_y);
  }
  friend Point operator* (const double& a, const Point& b)
  {
    return Point (a * b.m_x, a * b.m_y);
  }
  friend bool operator== (const Point& a, const Point& b)
  {
    return ((a.m_x == b.m_x) && (a.m_y == b.m_y));
  }
  friend bool operator!= (const Point& a, const Point& b)
  {
    return ((a.m_x != b.m_x) || (a.m_y != b.m_y));
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
    double val = (b.m_y - a.m_y) * (c.m_x - b.m_x) -
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

  Vector (double x = 0, double y = 0);
  Vector (const Point& a, const Point& b);
  Vector (const Point& p);
  double length() const;
  void normalize();

  friend std::string to_string(const Vector& v)
  {
    return "Vector(" + std::to_string(v.x()) + ";" + std::to_string(v.y()) + ")";
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
  friend bool operator== (const Vector& a, const Vector& b)
  {
    return a.x() == b.x() && a.y() == b.y();
  }
  friend bool operator!= (const Vector& a, const Vector& b)
  {
    return a.x() != b.x() || a.y() != b.y();
  }
};

class Line
{
  Point m_reference;
  Vector m_direction;
  
public:

  Line (const Point& a, const Point& b);
  Point projection (const Point& p) const;
  const Vector& direction() const;
};

class Segment
{
  Point m_source;
  Point m_target;

public:

  Segment (const Point& source, const Point& target);
  Vector to_vector() const;
  Line to_line() const;
  Box box() const;
  double length() const;
  double projected_coordinate (const Point& p) const;
  std::pair<Point, bool> projection (const Point& p) const;
  
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

    // We ignore colinear intersections (only point intersection are needed)
    return false;

    // Outdated incomplete code = does not work if one segment contains entirely the other
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

  friend Point intersection (const Segment& a, const Segment& b)
  {
    const Point& as = a.m_source;
    const Point& at = a.m_target;
    const Point& bs = b.m_source;
    const Point& bt = b.m_target;
    const double& x1 = as.x();
    const double& y1 = as.y();
    const double& x2 = at.x();
    const double& y2 = at.y();
    const double& x3 = bs.x();
    const double& y3 = bs.y();
    const double& x4 = bt.x();
    const double& y4 = bt.y();

    double D = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);
    check(D != 0, "Segments do not intersect");

    return Point (((x1*y2 - y1*x2) * (x3 - x4) - (x1 - x2) * (x3*y4 - y3*x4)) / D,
                  ((x1*y2 - y1*x2) * (y3 - y4) - (y1 - y2) * (x3*y4 - y3*x4)) / D);
  }
};

} // namespace Sosage

#endif // SOSAGE_UTILS_GEOMETRY_H
