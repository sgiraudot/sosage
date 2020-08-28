/*
  [src/Sosage/System/Animation.cpp]
  Generate animations and handle frame-by-frame image selection.

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

#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Config/config.h>
#include <Sosage/System/Animation.h>
#include <Sosage/Utils/random.h>

namespace Sosage::System
{

Animation::Animation (Content& content)
  : m_content (content), m_frame_id(0)
{
}

void Animation::run()
{
  std::size_t new_frame_id = m_content.get<Component::Int>(CLOCK__FRAME_ID)->value();

  auto status = m_content.get<Component::Status>(GAME__STATUS);
  if (status->value() == LOADING)
  {
#ifdef SOSAGE_THREADS_ENABLED
    for (std::size_t i = m_frame_id; i < new_frame_id; ++ i)
      m_content.get<Component::Animation>(LOADING_SPIN__IMAGE)->next_frame();
    m_frame_id = new_frame_id;
#endif
    return;
  }

  if (status->value() == PAUSED)
    return;

  if (new_frame_id == m_frame_id)
  {
    // Force update when new room is loaded
    if (m_content.request<Component::Boolean>("game:in_new_room"))
      run_one_frame();
    return;
  }

  for (std::size_t i = m_frame_id; i < new_frame_id; ++ i)
    run_one_frame();
  m_frame_id = new_frame_id;
}

void Animation::run_one_frame()
{
  if (auto new_char = m_content.request<Component::Vector<std::pair<std::string, bool> > >("game:new_characters"))
  {
    for (const auto& nc : new_char->value())
    {
      place_and_scale_character (nc.first, nc.second);
      generate_random_idle_animation (nc.first, nc.second);
    }
    m_content.remove("game:new_characters");
  }

  if (auto looking_right = m_content.request<Component::Boolean>("game:in_new_room"))
  {
    const std::string& player = m_content.get<Component::String>("player:name")->value();
    place_and_scale_character (player, looking_right->value());
    generate_random_idle_animation (player, looking_right->value());
    m_content.remove("game:in_new_room");
  }

  std::vector<std::string> to_remove;
  std::unordered_set<std::string> characters_talking;
  std::vector<Component::Animation_handle> animations;

  for (auto c : m_content)
  {
    if (auto path = Component::cast<Component::Path>(c))
    {
      if (!compute_movement_from_path(path))
        to_remove.push_back(c->id());
    }
    else if (c->component() == "lookat")
    {
      std::string id = c->entity();
      auto lookat = Component::cast<Component::Position>(c);
      auto abody = m_content.get<Component::Animation>(id + "_idle:image");
      auto ahead = m_content.get<Component::Animation>(id + "_head:image");
      auto pbody = m_content.get<Component::Position>(id + "_body:position");
      auto phead = m_content.get<Component::Position>(id + "_head:position");
      auto pmouth = m_content.get<Component::Position>(id + "_mouth:position");

      Vector direction (phead->value(), lookat->value());
      bool looking_right = (direction.x() > 0);

      if (looking_right)
      {
        phead->set (pbody->value() - abody->core().scaling
                    * Vector(m_content.get<Component::Position>(id + "_head:gap_right")->value()));
        pmouth->set (phead->value() - ahead->core().scaling
                     * Vector(m_content.get<Component::Position>(id + "_mouth:gap_right")->value()));
      }
      else
      {
        phead->set (pbody->value() - abody->core().scaling
                    * Vector(m_content.get<Component::Position>(id + "_head:gap_left")->value()));
        pmouth->set (phead->value() - ahead->core().scaling
                     * Vector(m_content.get<Component::Position>(id + "_mouth:gap_left")->value()));
      }

      generate_random_idle_animation (id, looking_right);

      // Avoid canceling mouth animation if character also starts talking now
      if (characters_talking.find(id) != characters_talking.end())
        generate_random_mouth_animation(id);

      to_remove.push_back (c->id());
    }
    else if (Component::cast<Component::Event>(c))
    {
      std::string id = c->entity();
      if (c->component() == "start_talking")
      {
        generate_random_mouth_animation (id);
        characters_talking.insert (id);
        to_remove.push_back(c->id());
      }
      else if (c->component() == "stop_talking")
      {
        generate_random_idle_head_animation (id,
                                             m_content.get<Component::Animation>(id + "_head:image")
                                             ->frames().front().y == 0);
        to_remove.push_back (c->id());
      }
      else if (c->component() == "stop_animation")
      {
        generate_random_idle_body_animation (id,
                                             m_content.get<Component::Animation>(id + "_head:image")
                                             ->frames().front().y == 0);
        to_remove.push_back (c->id());
      }
    }
    else if (c->component() == "start_animation")
    {
      auto anim = Component::cast<Component::String>(c);
      std::string id = c->entity();
      generate_animation (id, anim->value());
      to_remove.push_back (c->id());
    }
    else if (auto anim = Component::cast<Component::Animation>(c))
      if (anim->on())
        animations.push_back(anim);
  }

  for (const std::string& c : to_remove)
    m_content.remove(c);

  update_camera_target();

  for (const auto& animation : animations)
    if (!animation->next_frame())
      animation->on() = false;
}

void Animation::place_and_scale_character(const std::string& id, bool looking_right)
{
  auto abody = m_content.get<Component::Animation>(id + "_walking:image");
  auto aidle = m_content.get<Component::Animation>(id + "_idle:image");
  auto ahead = m_content.get<Component::Animation>(id + "_head:image");
  auto amouth = m_content.get<Component::Animation>(id + "_mouth:image");
  auto pbody = m_content.get<Component::Position>(id + "_body:position");
  auto phead = m_content.get<Component::Position>(id + "_head:position");
  auto pmouth = m_content.get<Component::Position>(id + "_mouth:position");
  auto ground_map = m_content.get<Component::Ground_map>("background:ground_map");

  double new_z = ground_map->z_at_point (pbody->value());
  abody->rescale (new_z);
  aidle->rescale (new_z);
  ahead->rescale (new_z);
  ahead->z() += 1;
  amouth->rescale (new_z);
  amouth->z() += 2;

  if (looking_right)
  {
    phead->set (pbody->value() - abody->core().scaling
                * Vector(m_content.get<Component::Position>(id + "_head:gap_right")->value()));
    pmouth->set (phead->value() - ahead->core().scaling
                * Vector(m_content.get<Component::Position>(id + "_mouth:gap_right")->value()));
  }
  else
  {
    phead->set (pbody->value() - abody->core().scaling
                * Vector(m_content.get<Component::Position>(id + "_head:gap_left")->value()));
    pmouth->set (phead->value() - ahead->core().scaling
                * Vector(m_content.get<Component::Position>(id + "_mouth:gap_left")->value()));
  }
}

bool Animation::compute_movement_from_path (Component::Path_handle path)
{
  bool out = true;

  std::string id = path->entity();
  auto abody = m_content.get<Component::Animation>(id + "_walking:image");
  auto aidle = m_content.get<Component::Animation>(id + "_idle:image");
  auto ahead = m_content.get<Component::Animation>(id + "_head:image");
  auto amouth = m_content.get<Component::Animation>(id + "_mouth:image");
  auto pbody = m_content.get<Component::Position>(id + "_body:position");
  auto phead = m_content.get<Component::Position>(id + "_head:position");
  auto pmouth = m_content.get<Component::Position>(id + "_mouth:position");
  auto ground_map = m_content.get<Component::Ground_map>("background:ground_map");

  Point pos = pbody->value();

  double to_walk = Config::character_speed * ground_map->z_at_point (pos) / Config::world_depth;

  Vector direction (pos, (*path)[path->current()]);
  direction.normalize();
  direction = to_walk * direction;
  direction = Vector (direction.x(), direction.y() / 3);
  to_walk = direction.length();

  while (true)
  {
    Vector current_vector (pos, (*path)[path->current()]);

    if (current_vector.length() > to_walk) // Next position is between current and current + 1
    {
      current_vector.normalize();
      pos = pos + to_walk * current_vector;
      set_move_animation(id, current_vector);
      break;
    }

    to_walk -= current_vector.length();
    pos = (*path)[path->current()];
    if (path->current() + 1 == path->size()) // Movement over, next position is target
    {
      pos = (*path)[path->current()];
      out = false;
      generate_random_idle_animation(id, current_vector.x() > 0);
      break;
    }
    path->current() ++;
  }

  pbody->set(pos);
  place_and_scale_character(id, direction.x() > 0);

  return out;
}

void Animation::set_move_animation (const std::string& id, const Vector& direction)
{
  auto image = m_content.get<Component::Animation>(id + "_walking:image");
  auto head = m_content.get<Component::Animation>(id + "_head:image");
  auto mouth = m_content.get<Component::Animation>(id + "_mouth:image");
  m_content.get<Component::Animation>(id + "_idle:image")->on() = false;

  image->on() = true;

  if (head->on())
  {
    image->reset();
    head->reset();
    head->on() = false;
  }
  if (mouth->on())
  {
    mouth->reset();
    mouth->on() = false;
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

void Animation::generate_random_idle_animation (const std::string& id, bool looking_right)
{
  generate_random_idle_body_animation (id, looking_right);
  generate_random_idle_head_animation (id, looking_right);
}

void Animation::generate_random_idle_head_animation (const std::string& id, bool looking_right)
{
  auto head = m_content.get<Component::Animation>(id + "_head:image");
  auto mouth = m_content.get<Component::Animation>(id + "_mouth:image");

  // Reset all
  head->reset();
  head->frames().clear();
  head->on() = true;

  // Reset all
  mouth->reset();
  mouth->frames().clear();
  mouth->on() = true;

  std::size_t row_index = 0;
  if (!looking_right)
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
      mouth->frames().push_back
        (Component::Animation::Frame
         (0, row_index, next_blink));

      if (remaining == 0)
        break;

      // Blink eyes
      head->frames().push_back
        (Component::Animation::Frame
         (1, row_index, 1));
      mouth->frames().push_back
        (Component::Animation::Frame
         (0, row_index, 1));;
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

    // TODO: handle change of gaps/reference when turning head (mouth must follow head)
#if 0
    // Change direction from time to time
    if (random_int(0,5) == 0)
      row_index = (row_index == 1 ? 0 : 1);
#endif
  }

}

void Animation::generate_random_idle_body_animation (const std::string& id, bool looking_right)
{
  auto image = m_content.get<Component::Animation>(id + "_idle:image");
  m_content.get<Component::Animation>(id + "_walking:image")->on() = false;

  // Reset all
  image->on() = true;
  image->reset();
  image->frames().clear();

  std::size_t row_index = 0;
  if (!looking_right)
    row_index = 1;

  std::vector<int> possibles_values;
  const std::vector<std::string>& positions
    = m_content.get<Component::Vector<std::string> >(id + "_idle:values")->value();

  int pose = 1;
  for (std::size_t i = 0; i < positions.size(); ++ i)
    if (positions[i] == "default")
    {
      possibles_values.push_back (int(i));
      pose = i;
    }
    else if (positions[i] == "idle")
      possibles_values.push_back (int(i));

  // Generate 10 poses with transitions

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
      new_pose = possibles_values[std::size_t(random_int(0, possibles_values.size()))];
    }
    while (new_pose == pose);
    pose = new_pose;
  }
}

void Animation::generate_random_mouth_animation (const std::string& id)
{
  auto image = m_content.get<Component::Animation>(id + "_mouth:image");
  // Reset all
  image->reset();
  image->frames().clear();
  image->on() = true;

  std::size_t row_index = (m_content.get<Component::Animation>(id + "_head:image")->frames().front().y);

  // Generate 50 poses
  int pose = random_int(1,11);
  for (int i = 0; i < 50; ++ i)
  {
    image->frames().push_back
      (Component::Animation::Frame
       (pose, row_index, 1));

    int new_pose;
    do
    {
      new_pose = random_int(1, 11);
    }
    while (new_pose == pose);
    pose = new_pose;

  }
}

void Animation::generate_animation (const std::string& id, const std::string& anim)
{
  auto image = m_content.get<Component::Animation>(id + "_idle:image");
  const std::vector<std::string>& positions
    = m_content.get<Component::Vector<std::string> >(id + "_idle:values")->value();

  std::size_t row_index = image->frames().front().y;

  // Reset all
  image->reset();
  image->frames().clear();


  int index = -1;
  for (std::size_t i = 0; i < positions.size(); ++ i)
    if (positions[i] == anim)
    {
      index = i;
      break;
    }
  check (index != -1, "No " + anim + " skin found for " + id);

  image->frames().push_back (Component::Animation::Frame (index, row_index, 1));
}

void Animation::update_camera_target ()
{
  const std::string& id = m_content.get<Component::String>("player:name")->value();
  int xbody = m_content.get<Component::Position>(id + "_body:position")->value().x();
  double xcamera = m_content.get<Component::Double>(CAMERA__POSITION)->value();

  if (xbody < xcamera + Config::camera_limit_left)
    m_content.get<Component::Double>("camera:target")->set (std::max (0, xbody - Config::camera_limit_right));
  else if (xbody > xcamera + Config::camera_limit_right)
  {
    int width = m_content.get<Component::Image>("background:image")->width();
    m_content.get<Component::Double>("camera:target")->set (std::min (width - Config::world_width, xbody - Config::camera_limit_left));
  }
}

} // namespace Sosage::System
