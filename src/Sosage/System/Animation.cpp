#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/System/Animation.h>
#include <Sosage/Config.h>
#include <Sosage/Utils/random.h>

namespace Sosage::System
{

Animation::Animation (Content& content)
  : m_content (content)
{

}

void Animation::main()
{
  Component::Path_handle path
    = m_content.request<Component::Path>("character:path");
  if (path)
    compute_movement_from_path(path);

  Component::Position_handle lookat
    = m_content.request<Component::Position>("character:lookat");
  if (lookat)
  {
    Component::Position_handle phead
      = m_content.get<Component::Position>("character_head:position");
    
    generate_random_idle_head_animation (m_content.get<Component::Animation>("character_head:image"),
                                         Vector (phead->value(), lookat->value()));
    m_content.remove("character:lookat");
  }

  std::vector<Component::Animation_handle> animations;

  for (const auto& e : m_content)
    if (Component::Animation_handle anim
        = Component::cast<Component::Animation>(e))
      animations.push_back(anim);

  for (const auto& animation : animations)
    animation->next_frame();
}

void Animation::compute_movement_from_path (Component::Path_handle path)
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
  ahead->z() += 1;
  pbody->set(pos);


  if (direction.x() > 0)
    phead->set (pbody->value() - abody->core().scaling
                * Vector(m_content.get<Component::Position>("character_head:gap_right")->value()));
  else
    phead->set (pbody->value() - abody->core().scaling
                * Vector(m_content.get<Component::Position>("character_head:gap_left")->value()));
      
}

void Animation::set_move_animation (Component::Animation_handle image,
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

  int angle = (int(180 * std::atan2 (direction.y(), direction.x()) / M_PI) + 360) % 360;

  const int limit = 18;
  if (angle < limit || angle >= (360 - limit))
    row_index = 1;
  else if (angle < (180 - limit))
    row_index = 0;
  else if (angle < (180 + limit))
    row_index = 2;
  else
    row_index = 3;

  image->frames().resize(8);
  for (std::size_t i = 0; i < 8; ++ i)
  {
    image->frames()[i].x = i;
    image->frames()[i].y = row_index;
    image->frames()[i].duration = 1;
  }
}

void Animation::generate_random_idle_animation (Component::Animation_handle image,
                                                Component::Animation_handle head,
                                                const Vector& direction)
{
  generate_random_idle_body_animation (image, direction);
  generate_random_idle_head_animation (head, direction);
}

void Animation::generate_random_idle_head_animation (Component::Animation_handle head,
                                                     const Vector& direction)
{
  // Reset all
  head->reset();
  head->frames().clear();
  head->on() = true;

  std::size_t row_index = 0;
  if (direction.x() < 0)
    row_index = 1;

  // Generate 10 poses with transitions
  int pose = 0;
  for (int i = 0; i < 10; ++ i)
  {
    int remaining = random_int(20, 150);

    while (true)
    {
      int next_blink = random_int(5, 50);

      if (remaining > next_blink)
        remaining -= next_blink;
      else
      {
        next_blink = remaining;
        remaining = 0;
      }

      // Stand still for a while
      head->frames().push_back
        (Component::Animation::Frame
         (pose, row_index, next_blink));

      if (remaining == 0)
        break;

      // Blink eyes
      head->frames().push_back
        (Component::Animation::Frame
         (1, row_index, 1));
    }

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

void Animation::generate_random_idle_body_animation (Component::Animation_handle image,
                                                     const Vector& direction)
{
  // Reset all
  image->reset();
  image->frames().clear();

  std::size_t row_index = 4;
  if (direction.x() < 0)
    row_index = 5;
  
  // Generate 10 poses with transitions
  int pose = 1;
  for (int i = 0; i < 10; ++ i)
  {
    // Stand still for a while
    image->frames().push_back
      (Component::Animation::Frame
       (pose, row_index, random_int(20, 150)));
    
    // Transition
    image->frames().push_back
      (Component::Animation::Frame
       (0, row_index, 2));
    
    int new_pose;
    do
    {
      new_pose = random_int(1,5);
    }
    while (new_pose == pose);
    pose = new_pose;
  }
}

} // namespace Sosage::System
