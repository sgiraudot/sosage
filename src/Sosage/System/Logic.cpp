#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
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
  if (!m_chosen_verb)
    m_chosen_verb = m_content.get<Component::Text> ("verb_goto:text");
  
  Component::Path_handle path
    = m_content.request<Component::Path>("character:path");
  if (path)
    compute_movement_from_path(path);

  Component::Position_handle mouse
    = m_content.request<Component::Position>("mouse:position");
  if (mouse)
    detect_collision (mouse);

  Component::Position_handle clicked
    = m_content.request<Component::Position>("mouse:clicked");
  if (clicked && m_collision)
  {
    std::cerr << m_collision->id() << std::endl;
    if (m_collision->id() == "background:image")
    {
      compute_path_from_target(clicked);
      m_chosen_verb = m_content.get<Component::Text> ("verb_goto:text");
    }
    else if (m_collision->entity().find("verb_") == 0)
      m_chosen_verb = m_content.get<Component::Text> (m_collision->entity() + ":text");
    else if (Component::Text_handle name = m_content.request<Component::Text>(m_collision->entity() + ":name"))
    {
      if (m_chosen_verb->entity() == "verb_goto")
        compute_path_from_target(m_content.get<Component::Position>(m_collision->entity() + ":position"));
      else
      {
        // Action !
#if 0
        if (Component::Action_handle action = m_content.request<Component::Action>
            (m_collision->entity() + ":" + m_chosen_verb->entity()))
        {

        }
#endif
      }
    }

    m_content.remove("mouse:clicked");
  }

  update_interface();
  
  update_debug_info (m_content.get<Component::Debug>("game:debug"));
}

bool Logic::exit()
{
  Component::Boolean_handle exit
    = m_content.request<Component::Boolean>("game:exit");
  if (exit && exit->value())
    return true;
  return false;
}

bool Logic::paused()
{
  return m_content.get<Component::Boolean>("game:paused")->value();
}

void Logic::compute_path_from_target (Component::Position_handle target)
{
  std::vector<Point> path;
  
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
    int remaining = random_int(10, 150);

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
         (pose, row_index, next_blink * config().animation_frame_rate));

      if (remaining == 0)
        break;

      // Blink eyes
      head->frames().push_back
        (Component::Animation::Frame
       (1, row_index, config().animation_frame_rate));
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

void Logic::detect_collision (Component::Position_handle mouse)
{
  // Deactive previous collisions
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(0.75);
  }

  m_collision = Component::Image_handle();
  
  for (const auto& e : m_content)
    if (Component::Image_handle img
        = Component::cast<Component::Image>(e))
    {
      if (!img->on() ||
          img->id().find("character") == 0 ||
          img->id().find("debug") == 0)
        continue;
      
      Component::Position_handle p = m_content.get<Component::Position>(img->entity() + ":position");

      Point screen_position = p->value() - img->core().second * Vector(img->origin());
      int xmin = screen_position.x();
      int ymin = screen_position.y();
      int xmax = xmin + (img->core().second * (img->xmax() - img->xmin()));
      int ymax = ymin + (img->core().second * (img->ymax() - img->ymin()));

      if (mouse->value().x() < xmin ||
          mouse->value().x() > xmax ||
          mouse->value().y() < ymin ||
          mouse->value().y() > ymax)
        continue;

      // Now, collision happened
      
      if (m_collision)
      {
        // Keep image closest to screen
        if (img->z() > m_collision->z())
          m_collision = img;
      }
      else
        m_collision = img;

    }

}

void Logic::update_interface ()
{
  std::string target_object = "";
  
  if (m_collision)
  {
    if (m_collision->entity().find("verb_") == 0)
      m_collision->set_scale(1.0);
    if (Component::Text_handle name = m_content.request<Component::Text>(m_collision->entity() + ":name"))
      target_object = name->value();
  }

  Component::Image_handle text_img
    = m_content.set<Component::Image>("chosen_verb:image",
                                      m_content.get<Component::Font>("interface:font"), "FFFFFF",
                                      m_chosen_verb->value() + " " + target_object);
  text_img->origin() = Point (text_img->width() / 2, text_img->height() / 2);
  text_img->set_scale(0.5);
}

void Logic::update_debug_info (Component::Debug_handle debug_info)
{
  if (debug_info->value())
  {
    Component::Font_handle debug_font
      = m_content.get<Component::Font> ("debug:font");
    Component::Image_handle dbg_img
      = m_content.set<Component::Image> ("debug:image",
                                         debug_font, "FF0000",
                                         debug_info->debug_str());
    Component::Position_handle dbg_pos
      = m_content.set<Component::Position>("debug:position", Point(0,0));
  }
  else
  {
    Component::Image_handle dbg_img = m_content.request<Component::Image> ("debug:image");
    if (dbg_img)
      dbg_img->on() = false;
    
  }

}

} // namespace Sosage::System
