#ifndef SOSAGE_COMPONENT_GROUND_MAP_H
#define SOSAGE_COMPONENT_GROUND_MAP_H

#include <Sosage/Component/Handle.h>
#include <Sosage/Utils/geometry.h>
#include <Sosage/Utils/vector_2.h>
#include <Sosage/third_party_config.h>

namespace Sosage::Component
{

class Ground_map : public Base
{
  typedef Graphic_core Core;

  struct Node
  {
    double z;
    int target_x;
    int target_y;

    int prev_x;
    int prev_y;
    int distance;
    
    Node() : z(0.), target_x(-1), target_y(-1), prev_x(-1), prev_y(-1), distance(0) { }
  };

  class Sort_by_distance
  {
    const vector_2<Node>& m_data;

  public:
    Sort_by_distance (const vector_2<Node>& data) : m_data (data) { }

    bool operator() (const std::pair<int, int>& a, const std::pair<int, int>& b) const
    {
      int da = m_data(a.first, a.second).distance;
      int db = m_data(b.first, b.second).distance;
      if (da == db)
        return a < b;
      return da < db;
    }
  };

  vector_2<Node> m_data;
  
public:

  Ground_map (const std::string& file_name);

  void find_path (const Point& origin, const Point& target,
                  std::vector<Point>& out);

};
typedef std::shared_ptr<Ground_map> Ground_map_handle;

} // namespace Sosage::Component

#endif // SOSAGE_COMPONENT_GROUND_MAP_H
