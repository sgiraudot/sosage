#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/System/Logic.h>
#include <Sosage/Utils/geometry.h>

#include <vector>

namespace Sosage::System
{

Logic::Logic (Content& content)
  : m_content (content)
{

}

void Logic::main()
{
  std::vector<Point> path;
  
  while (this->running())
  {
    Component::Path_handle target
      = m_content.get<Component::Path>("character", "target_query");
    if (target)
    {
      target->lock();

      m_content.remove("character", "target_query");
      
      Component::Ground_map_handle ground_map
        = m_content.get<Component::Ground_map>("background", "ground_map");
      if (!ground_map)
        error ("Cannot find ground map");

      ground_map->lock();

      Component::Path_handle position
        = m_content.get<Component::Path>("character", "position");
      Component::Image_handle image
        = m_content.get<Component::Image>("character", "image");
      
      if (!ground_map)
        error ("Cannot find character position");

      position->lock();
      image->lock();

      Point origin = (*position)[0];
      origin = origin + Vector (Graphic_core::width(image->core()) / 2,
                                Graphic_core::height(image->core()), CAMERA);

      // Todo find_path
      ground_map->find_path (origin, (*target)[0], path);
      m_content.set<Component::Path>("character", "path", path);
      path.clear();
      
      position->unlock();
      image->unlock();
      ground_map->unlock();
      target->unlock();
    }

    Component::Path_handle path
      = m_content.get<Component::Path>("character", "path");
    if (path)
    {
      path->lock();

      Component::Path_handle position
        = m_content.get<Component::Path>("character", "position");
      Component::Image_handle image
        = m_content.get<Component::Image>("character", "image");

      Component::Ground_map_handle ground_map
        = m_content.get<Component::Ground_map>("background", "ground_map");
      if (!ground_map)
        error ("Cannot find ground map");

      ground_map->lock();

      position->lock();
      image->lock();

      Vector translation (Graphic_core::width(image->core()) / 2,
                          Graphic_core::height(image->core()), CAMERA);
      Point pos = (*position)[0] + translation;

      double to_walk = config().character_speed;
      while (true)
      {
        Vector current_vector (pos, (*path)[path->current() + 1]);
        
        if (current_vector.length() > to_walk) // Next position is between current and current + 1
        {
          current_vector.normalize();
          pos = pos + to_walk * current_vector;
          break;
        }

        to_walk -= current_vector.length();
        path->current() ++;
        if (path->current() + 1 == path->size()) // Movement over, next position is target
        {
          pos = (*path)[path->current()];
          m_content.remove("character", "path");
          break;
        }
      }
      (*position)[0] = pos - translation;
      
      position->unlock();
      image->unlock();
      ground_map->unlock();
      path->unlock();
    }
    
    this->wait();
  }
}

} // namespace Sosage::System
