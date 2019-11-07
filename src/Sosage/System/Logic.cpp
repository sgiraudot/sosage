#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Logic.h>
#include <Sosage/Utils/geometry.h>
#include <Sosage/Utils/random.h>

#include <vector>

namespace Sosage::System
{

Logic::Logic (Content& content)
  : m_content (content)
{

}

void Logic::main()
{
  Component::Position_handle target
    = m_content.request<Component::Position>("character:target_query");
  if (target)
    compute_path_from_target(target);

  Component::Path_handle path
    = m_content.request<Component::Path>("character:path");
  if (path)
    compute_movement_from_path(path);
}

bool Logic::exit()
{
  Component::Boolean_handle exit
    = m_content.request<Component::Boolean>("game:exit");
  if (exit && exit->value())
    return true;
  return false;
}

void Logic::compute_path_from_target (Component::Position_handle target)
{
  std::vector<Point> path;
  
  m_content.remove("character:target_query");
      
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");

  Component::Animation_handle image
    = m_content.get<Component::Animation>("character_body:image");
      
  Component::Position_handle position
    = m_content.get<Component::Position>("character_body:position");
      
  Point origin = position->value();

  ground_map->find_path (origin, target->value(), path);
      
  m_content.set<Component::Path>("character:path", path);
  path.clear();
      
}

void Logic::compute_movement_from_path (Component::Path_handle path)
{
  Component::Animation_handle abody
    = m_content.get<Component::Animation>("character_body:image");

  Component::Animation_handle ahead
    = m_content.get<Component::Animation>("character_head:image");

  Component::Position_handle pbody
    = m_content.get<Component::Position>("character_body:position");
  
  Component::Position_handle phead
    = m_content.get<Component::Position>("character_head:position");

  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");

  Point pos = pbody->value();

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
      set_move_animation(abody, ahead, current_vector);
      break;
    }

    to_walk -= current_vector.length();
    pos = (*path)[path->current()];
    if (path->current() + 1 == path->size()) // Movement over, next position is target
    {
      pos = (*path)[path->current()];
      m_content.remove("character:path");
      generate_random_idle_animation(abody, ahead, current_vector);
      break;
    }
    path->current() ++;
  }

  double new_z = ground_map->z_at_point (pos);
  abody->rescale (new_z);
  ahead->rescale (new_z);
  pbody->set(pos);


  if (direction.x() > 0)
    phead->set (pbody->value() - abody->core().second * Vector(0, 290));
  else
    phead->set (pbody->value() - abody->core().second * Vector(-10, 290));

      
}

void Logic::set_move_animation (Component::Animation_handle image,
                                Component::Animation_handle head,
                                const Vector& direction)
{
  if (head->on())
  {
    image->reset();
    head->reset();
    head->on() = false;
  }
  std::size_t row_index = 0;
  if (std::abs(direction.x()) > std::abs(direction.y()))
  {
    if (direction.x() > 0)
      row_index = 1;
    else
      row_index = 2;
  }
  else
  {
    if (direction.y() > 0)
      row_index = 0;
    else
      row_index = 3;
  }

  image->frames().resize(8);
  for (std::size_t i = 0; i < 8; ++ i)
  {
    image->frames()[i].x = i+1;
    image->frames()[i].y = row_index;
    image->frames()[i].duration = config().animation_frame_rate;
  }
}

void Logic::generate_random_idle_animation (Component::Animation_handle image,
                                            Component::Animation_handle head,
                                            const Vector& direction)
{
  // Reset all
  image->reset();
  head->reset();
  head->on() = true;

  std::size_t row_index = 0;
  if (direction.x() > 0)
    row_index = 1;
  else
    row_index = 2;
  
  // Stand still for a while
  image->frames().push_back
    (Component::Animation::Frame
     (0, row_index, random_int(10, 150) * config().animation_frame_rate));

  std::size_t offset = 0;
  if (direction.x() < 0)
    offset = 4;
  
  // Generate 10 poses with transitions
  int pose = 0;
  for (int i = 0; i < 10; ++ i)
  {
    // Stand still for a while
    if (pose == 0)
      image->frames().push_back
        (Component::Animation::Frame
         (0, row_index, random_int(10, 150) * config().animation_frame_rate));
    else
      image->frames().push_back
        (Component::Animation::Frame
         (offset + pose, 4, random_int(10, 150) * config().animation_frame_rate));
    
    // Transition
    image->frames().push_back
      (Component::Animation::Frame
       (offset, 4, 2 * config().animation_frame_rate));
    int new_pose;
    do
    {
      new_pose = random_int(0,4);
    }
    while (new_pose == pose);
    pose = new_pose;
  }

  if (row_index == 1)
    row_index = 0;
  else
    row_index = 1;

  // Same for the head
  pose = 0;
  for (int i = 0; i < 10; ++ i)
  {
    // Stand still for a while
    head->frames().push_back
      (Component::Animation::Frame
       (pose, row_index, random_int(10, 50) * config().animation_frame_rate));

    head->frames().push_back
      (Component::Animation::Frame
       (1, row_index, config().animation_frame_rate));

    if (random_int(0,4) == 0)
    {
      int new_pose;
      do
      {
        new_pose = random_int(2,6);
      }
      while (new_pose == pose);
      pose = new_pose;
    }

    // Change direction from time to time
    if (random_int(0,5) == 0)
      row_index = (row_index == 1 ? 0 : 1);
  }

}

  

} // namespace Sosage::System
