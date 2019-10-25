#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Status.h>
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
  while (this->running())
  {
    if (exit())
    {
      this->stop();
      break;
    }
    
    Component::Path_handle target
      = m_content.request<Component::Path>("character:target_query");
    if (target)
      compute_path_from_target(target);

    Component::Path_handle path
      = m_content.request<Component::Path>("character:path");
    if (path)
      compute_movement_from_path(path);
    
    this->wait();
  }
}

bool Logic::exit()
{
  bool out = false;
  Component::Status_handle status
    = m_content.request<Component::Status>("game:status");
  if (status)
  {
    status->lock();
    if (status->exit())
      out = true;
    status->unlock();
  }
  return out;
}

void Logic::compute_path_from_target (Component::Path_handle target)
{
  std::vector<Point> path;
  
  target->lock();

  m_content.remove("character:target_query");
      
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");

  Component::Animation_handle image
    = m_content.get<Component::Animation>("character:image");
  image->lock();
      
  Component::Path_handle position
    = m_content.get<Component::Path>("character:position");
  position->lock();
      
  Point origin = (*position)[0];
  origin = origin + Vector (image->width() / 2,
                            image->height(), CAMERA);

  ground_map->find_path (origin, (*target)[0], path);
      
  m_content.set<Component::Path>("character:path", path);
  path.clear();
      
  position->unlock();
  image->unlock();
  target->unlock();
}

void Logic::compute_movement_from_path (Component::Path_handle path)
{
  path->lock();

  Component::Animation_handle image
    = m_content.get<Component::Animation>("character:image");
  image->lock();

  Component::Animation_handle head
    = m_content.get<Component::Animation>("character:head");
  head->lock();

  Component::Path_handle position
    = m_content.get<Component::Path>("character:position");
  position->lock();

  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");

  Vector translation (image->width() / 2,
                      image->height(), CAMERA);
  Point pos = (*position)[0] + translation;

  double to_walk = config().character_speed * ground_map->z_at_point (pos) / config().world_depth;
      
  Vector direction (pos, (*path)[path->current()]);
  direction.normalize();
  direction = to_walk * direction;
  direction = Vector (direction.x(), direction.y() / 3, WORLD);
  to_walk = direction.length();

  while (true)
  {
    Vector current_vector (pos, (*path)[path->current()]);
        
    if (current_vector.length() > to_walk) // Next position is between current and current + 1
    {
      current_vector.normalize();
      pos = pos + to_walk * current_vector;
      set_move_animation(image, head, current_vector);
      break;
    }

    to_walk -= current_vector.length();
    pos = (*path)[path->current()];
    if (path->current() + 1 == path->size()) // Movement over, next position is target
    {
      pos = (*path)[path->current()];
      m_content.remove("character:path");
      stop_move_animation(image, head);
      break;
    }
    path->current() ++;
  }

  int new_z = ground_map->z_at_point (pos);
  if (std::abs(image->z() - new_z) > image->z() / 10)
  {
    image->rescale (new_z);
    head->rescale (new_z);
  }
  Vector back_translation (image->width() / 2,
                           image->height(), CAMERA);
  (*position)[0] = pos - back_translation;
      
  position->unlock();
  head->unlock();
  image->unlock();
  path->unlock();
}

void Logic::set_move_animation (Component::Animation_handle image,
                                Component::Animation_handle head,
                                const Vector& direction)
{
  std::size_t row_index = 0;
  
  if (std::abs(direction.x()) > std::abs(direction.y()))
  {
    if (direction.x() > 0)
      row_index = 3;
    else
      row_index = 4;
  }
  else
  {
    if (direction.y() > 0)
      row_index = 5;
    else
      row_index = 2;
  }

  image->frames().resize(6);
  for (std::size_t i = 0; i < 6; ++ i)
  {
    image->frames()[i].x = i;
    image->frames()[i].y = row_index;
    image->frames()[i].duration = 1;
  }

  head->on() = false;
}

void Logic::stop_move_animation (Component::Animation_handle image,
                                 Component::Animation_handle head)
{
  image->reset();
  head->on() = true;
}

} // namespace Sosage::System
