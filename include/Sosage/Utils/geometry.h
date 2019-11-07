#ifndef SOSAGE_UTILS_GEOMETRY_H
#define SOSAGE_UTILS_GEOMETRY_H

#include <Sosage/Config.h>

#include <cmath>
#include <iostream>

namespace Sosage
{

enum Space { WORLD, GROUND };

template <typename T>
inline T square (const T& t) { return t*t; }

inline double distance (double xa, double ya, double xb, double yb)
{
  return std::sqrt (square(xa - xb) + square(ya - yb));
}

class Vector;

class Point
{
  double m_x;
  double m_y;
  
public:

  Point (double x, double y, const Space& space = WORLD)
  {
    m_x = from(x,space);
    m_y = from(y,space);
  }

  Point (const std::pair<double, double>& coord, const Space& space = WORLD)
    : Point (coord.first, coord.second, space)
  { }

  double from (const double& v, const Space& space) const
  {
    if (space == WORLD)
      return v;
    // else
    return (v + 0.5) * config().ground_map_scaling;
  }
  
  double to (const double& v, const Space& space) const
  {
    if (space == WORLD)
      return v;
    // else
    return (v / config().ground_map_scaling) - 0.5;
  }
  
  double convert (const double& v, const Space& f, const Space& t) const
  {
    return to(from(v,f),t);
  }
  
  double x(const Space& space = WORLD) const { return to(m_x,space); }
  double y(const Space& space = WORLD) const { return to(m_y,space); }

  friend std::ostream& operator<< (std::ostream& os, const Point& c)
  {
    os << "(" << c.m_x << ";" << c.m_y << ")";
    return os;
  }

  friend Point operator+ (const Point& a, const Point& b)
  {
    return Point (a.m_x + b.m_x, a.m_y + b.m_y, WORLD);
  }
  friend Point operator* (const double& a, const Point& b)
  {
    return Point (a * b.m_x, a * b.m_y, WORLD);
  }
};

class Vector : public Point
{
public:

  Vector (double x, double y, const Space& space = WORLD)
    : Point (x, y, space)
  { }

  Vector (const Point& a, const Point& b)
    : Point (b.x() - a.x(), b.y() - a.y(), WORLD)
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
    *this = Vector(x() / l, y() / l, WORLD);
  }

  friend Point operator+ (const Point& a, const Vector& b)
  {
    return Point(a.x() + b.x(), a.y() + b.y(), WORLD);
  }
  friend Point operator- (const Point& a, const Vector& b)
  {
    return Point(a.x() - b.x(), a.y() - b.y(), WORLD);
  }
  friend double operator* (const Vector& a, const Vector& b)
  {
    return a.x() * b.x() + a.y() * b.y();
  }
  friend Vector operator* (const double& a, const Vector& b)
  {
    return Vector (a * b.x(), a * b.y(), WORLD);
  }
};

inline double distance (const Point& a, const Point& b)
{
  return distance (a.x(WORLD), a.y(WORLD),
                   b.x(WORLD), b.y(WORLD));
}

} // namespace Sosage

#endif // SOSAGE_UTILS_GEOMETRY_H
