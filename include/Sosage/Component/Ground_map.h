#ifndef SOSAGE_COMPONENT_GROUND_MAP_H
#define SOSAGE_COMPONENT_GROUND_MAP_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/geometry.h>
#include <Sosage/Utils/vector_2.h>

namespace Sosage::Component
{

class Ground_map : public Base
{
  struct Node
  {
    int z;
    int target_x;
    int target_y;

    int prev_x;
    int prev_y;
    double distance;
    
    Node() : z(0.), target_x(-1), target_y(-1), prev_x(-1), prev_y(-1), distance(0.) { }
  };

  class Sort_by_distance
  {
    const vector_2<Node>& m_data;

  public:
    Sort_by_distance (const vector_2<Node>& data) : m_data (data) { }

    bool operator() (const std::pair<int, int>& a, const std::pair<int, int>& b) const
    {
      double da = m_data(a.first, a.second).distance;
      double db = m_data(b.first, b.second).distance;
      if (da == db)
        return a < b;
      return da < db;
    }
  };

  vector_2<Node> m_data;

public:

  Ground_map (const std::string& id, const std::string& file_name);

  void find_path (const Point& origin, const Point& target,
                  std::vector<Point>& out);

  double z_at_point (const Point& p) const;

private:

  double distance (int xa, int ya, int xb, int yb)
  {
    Point pa (xa, ya, GROUND);
    Point pb (xb, yb, GROUND);
    double za = m_data(xa,ya).z;
    double zb = m_data(xb,yb).z;

    return std::sqrt (square(pa.x() - pb.x())
                      + square(pa.y() - pb.y())
                      + square(za - zb));
  }
};
typedef std::shared_ptr<Ground_map> Ground_map_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GROUND_MAP_H
