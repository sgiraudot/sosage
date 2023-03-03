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
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Group.h>
#include <Sosage/Component/GUI_animation.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Config/config.h>
#include <Sosage/System/Animation.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/helpers.h>

namespace Sosage::System
{

namespace C = ::Sosage::Component;

Animation::Animation (Content& content)
  : Base (content), m_frame_id(0)
{
}

void Animation::run()
{
  SOSAGE_TIMER_START(System_Animation__run);
  SOSAGE_UPDATE_DBG_LOCATION("Animation::run()");

  std::size_t new_frame_id = frame_id(value<C::Double>(CLOCK__TIME));

  if (status()->is (PAUSED))
  {
    m_frame_id = new_frame_id;
    SOSAGE_TIMER_STOP(System_Animation__run);
    return;
  }

  run_gui_frame();

  if (status()->is (IN_MENU))
  {
    m_frame_id = new_frame_id;
    SOSAGE_TIMER_STOP(System_Animation__run);
    return;
  }

  if (new_frame_id == m_frame_id)
  {
    // Force update when new room is loaded
    if (signal("Game", "in_new_room"))
      run_animation_frame();
    SOSAGE_TIMER_STOP(System_Animation__run);
    return;
  }

  for (std::size_t i = m_frame_id; i < new_frame_id; ++ i)
    run_animation_frame();
  m_frame_id = new_frame_id;
  SOSAGE_TIMER_STOP(System_Animation__run);
}

void Animation::run_gui_frame()
{
  if (auto camera_fade = request<C::Tuple<double, double, bool>>("Camera", "fade"))
  {
    if (!fade (camera_fade->get<0>(), camera_fade->get<1>(), camera_fade->get<2>()))
    {
      remove(camera_fade);
      if (camera_fade->get<2>())
        get<C::Image>("Blackscreen", "image")->on() = false;
    }
  }

  double current_time = value<C::Double>(CLOCK__TIME);
  if (auto shake = request<C::Array<double,4>>("Camera", "shake"))
  {
    const double& begin = (*shake)[0];
    const double& end= (*shake)[1];
    const double& intensity = (*shake)[2];
    const double& x_start = (*shake)[3];

    double current_intensity = intensity * (end - current_time) / (end - begin);

    constexpr double period = 0.02;

    double shift = std::sin ((current_time - begin) / period);

    get<C::Absolute_position>(CAMERA__POSITION)->set
        (Point(x_start + shift * current_intensity, 0));
  }

  std::vector<C::Handle> to_remove;
  for (auto c : components("animation"))
    if (auto a = C::cast<C::GUI_animation>(c))
      if (!a->update(current_time))
        to_remove.emplace_back(a);

  for (auto c : components("move60fps"))
    if (auto a = C::cast<C::Tuple<Point, Point, double, double>>(c))
    {
      double ftime = current_time;
      double ratio = (ftime - a->get<2>()) / (a->get<3>() - a->get<2>());

      Point current = ratio * a->get<1>() + (1 - ratio) * a->get<0>();
      if (ratio > 1)
      {
        current = a->get<1>();

        // TODO improve scheduled action so that it's not needed
        if (c->entity() == "Camera")
          to_remove.emplace_back(a);
      }
      get<C::Position>(a->entity() , "position")->set (current);
    }

  for (auto tr : to_remove)
  {
    if (auto a = C::cast<C::GUI_animation>(tr))
    {
      if (a->remove_after())
      {
        const C::Id& object_id = a->object_id();
        remove(object_id.first, object_id.second);
        if (object_id.second == "image")
          remove(object_id.first, "position");
      }
    }
    remove(tr);
  }

  for (auto c : components("rescale60fps"))
    if (auto a = C::cast<C::Tuple<double, double, double, double>>(c))
    {
      double ftime = current_time;
      double ratio = (ftime - a->get<2>()) / (a->get<3>() - a->get<2>());

      double scale = ratio * a->get<1>() + (1 - ratio) * a->get<0>();
      if (ratio > 1)
        scale = a->get<1>();

      get<C::Image>(a->entity() , "image")->set_scale (scale);
    }

}

void Animation::run_animation_frame()
{
  bool in_new_room = receive ("Game", "in_new_room");
  if (in_new_room)
  {
    emit("Music", "adjust_mix");

    // Relaunch animations
    for (auto c : components("image"))
      if (request<C::String>(c->entity() , "state"))
        if (auto anim = C::cast<C::Animation>(c))
          anim->on() = true;
  }

  std::vector<C::Handle> to_remove;
  std::vector<C::Animation_handle> animations;

  // First check if some character should change looking direction
  for (auto c : components("lookat"))
  {
    const std::string& id = c->entity();
    auto lookat = C::cast<C::Position>(c);
    debug << "lookat " << lookat->str() << std::endl;
    if (in_new_room)
    {
      debug << "Place and scale from " << get<C::Position>(id, "position")->str() << std::endl;
      place_and_scale_character (id);
    }

    auto abody = get<C::Animation>(id + "_body", "image");
    auto ahead = get<C::Animation>(id + "_head", "image");
    auto pbody = get<C::Position>(id + "_body", "position");
    auto phead = get<C::Position>(id + "_head", "position");
    auto pmouth = get<C::Position>(id + "_mouth", "position");

    to_remove.push_back (c);

    Vector direction (pbody->value(), lookat->value());
    bool looking_right = (direction.x() > 0);

    if (!in_new_room && looking_right == is_looking_right(id))
      continue;

    generate_random_idle_animation (id, looking_right);
  }

  // Then check animations stopping
  for (auto c : components("stop_talking"))
  {
    const std::string& id = c->entity();
    if (auto mhead = request<C::Position>(id + "_head_move", "position"))
    {
      mhead->set (Point (0, 0));
      generate_random_idle_head_animation (id, is_looking_right(id));
    }
    to_remove.push_back (c);
  }

  for (auto c : components("stop_walking"))
  {
    const std::string& id = c->entity();
    if (value<C::Boolean>(id , "walking"))
    {
      generate_random_idle_animation (id, is_walking_right(id));
      place_and_scale_character(id);
    }
    to_remove.push_back (c);
  }

  for (auto c : components("stop_animation"))
  {
    const std::string& id = c->entity();
    debug << "stop_animation" << std::endl;
    if (request<C::Animation>(id + "_head", "image"))
      generate_random_idle_body_animation (id, is_looking_right(id));
    else
      get<C::Animation>(id , "image")->on() = false;
    to_remove.push_back (c);
  }

  for (auto c : components("pause"))
  {
    const std::string& id = c->entity();
    get<C::Animation>(id, "image")->playing() = false;
    to_remove.push_back (c);
  }

  bool has_moved = false;

  std::unordered_set<std::string> just_started;

  // Then check all other cases
  for (auto c : components("move"))
    if (auto a = C::cast<C::Tuple<Point, Point, int, int, double, double>>(c))
    {
      double ftime = frame_time(value<C::Double>(CLOCK__TIME));
      double ratio = (ftime - a->get<4>()) / (a->get<5>() - a->get<4>());

      Point current = ratio * a->get<1>() + (1 - ratio) * a->get<0>();
      if (ratio > 1)
        current = a->get<1>();
      int z = round(ratio * a->get<3>() + (1 - ratio) * a->get<2>());
      if (ratio > 1)
        z = a->get<3>();

      get<C::Position>(a->entity() , "position")->set (current);
      get<C::Image>(a->entity(), "image")->z() = z;
    }

  for (auto c : components("rescale"))
    if (auto a = C::cast<C::Tuple<double, double, double, double>>(c))
    {
      double ftime = frame_time(value<C::Double>(CLOCK__TIME));
      double ratio = (ftime - a->get<2>()) / (a->get<3>() - a->get<2>());

      double scale = ratio * a->get<1>() + (1 - ratio) * a->get<0>();
      if (ratio > 1)
        scale = a->get<1>();

      get<C::Image>(a->entity() , "image")->set_scale (scale);
    }


  for (auto c : components("path"))
    if (auto path = C::cast<C::Path>(c))
    {
      if (path->entity() != "Debug" && !compute_movement_from_path(path))
      {
        to_remove.push_back(c);
        if (auto speed_factor = request<C::Double>(c->entity(), "speed_factor"))
          to_remove.push_back(speed_factor);
      }
      else if (path->entity() == value<C::String>("Player", "name"))
      {
        has_moved = true;
        emit("Music", "adjust_mix");
      }
    }

  for (auto c : components("start_talking"))
  {
    const std::string& id = c->entity();
    generate_random_mouth_animation (id);
    to_remove.push_back(c);
  }

  for (auto c : components("start_animation"))
  {
    const std::string& id = c->entity();
    if (auto s = C::cast<C::Signal>(c))
    {
      auto anim = get<C::Animation>(id , "image");
      anim->on() = true;
      anim->playing() = true;
      just_started.insert (id);
      to_remove.push_back(c);
    }
    else
    {
      auto anim = C::cast<C::String>(c);
      const std::string& id = c->entity();
      debug << "start_animation" << std::endl;
      generate_animation (id, anim->value());
      to_remove.push_back (c);
    }
  }

  for (auto c : components("set_state"))
  {
    auto state = C::cast<C::String>(c);
    debug << "Set state of " << state->entity() << " to " << state->value() << std::endl;
    get<C::String>(state->entity(), "state")->set(state->value());
    to_remove.push_back(c);
  }

  for (auto c : components("set_hidden"))
  {
    debug << "Set " << c->str() << " hidden" << std::endl;
    const std::string& id = c->entity();
    auto g = get<C::Group>(id , "group");
    g->apply<C::Image>([](auto img) { img->on() = false; });
    to_remove.push_back(c);
  }

  for (auto c : components("set_visible"))
  {
    debug << "Set " << c->str() << " visible" << std::endl;
    const std::string& id = c->entity();
    auto g = get<C::Group>(id , "group");
    g->apply<C::Image>([](auto img) { img->on() = true; });
    to_remove.push_back(c);
  }

  for (auto c : components("image"))
    if (auto anim = C::cast<C::Animation>(c))
      if (anim->on() && anim->playing())
      {
        animations.push_back(anim);

        if (anim->animated())
        {
          // Randomly move head if character speaking
          auto pos = anim->entity().find("_mouth");
          if (pos != std::string::npos)
          {
            if (random_int(0,3) == 0)
            {
              std::string entity (anim->entity().begin(), anim->entity().begin() + pos);
              auto mhead = get<C::Position>(entity + "_head_move", "position");

              int radius = value<C::Int>(entity, "head_move_radius", 4);
              double direction = random_double(0, 2 * M_PI);
              double length = random_double (radius / 2., radius);
              mhead->set (Point (length * std::cos(direction), length * std::sin(direction)));
            }
          }
        }
      }

  for (C::Handle c : to_remove)
    remove(c);

  if (has_moved)
    update_camera_target();

  for (const auto& animation : animations)
    if (!contains(just_started, animation->entity()))
      if (!animation->next_frame())
        animation->on() = false;
}

bool Animation::run_loading()
{
  std::size_t new_frame_id = frame_id(value<C::Double>(CLOCK__TIME));
  if (new_frame_id == m_frame_id)
    return false;
  m_frame_id = new_frame_id;
  m_content.get<Component::Animation>("Loading_spin", "image")->next_frame();
  return true;
}

void Animation::place_and_scale_character(const std::string& id)
{
  // If character has no skin ("fake" character), do nothing
  if (!request<C::Group>(id, "group"))
    return;

  auto abody = get<C::Animation>(id + "_body", "image");
  auto ahead = get<C::Animation>(id + "_head", "image");
  auto amouth = get<C::Animation>(id + "_mouth", "image");
  auto mask = request<C::Image>(id, "image");
  auto pbody = get<C::Position>(id + "_body", "position");

  auto ground_map = get_ground_map(id);
  auto z = request<C::Int>(id , "z");
  if (z || !ground_map)
  {
    abody->rescale (Config::world_depth);
    ahead->rescale (Config::world_depth);
    amouth->rescale (Config::world_depth);
    if (mask)
      mask->rescale (Config::world_depth + 1);

    if (z)
    {
      abody->z() = z->value();
      ahead->z() = z->value();
      amouth->z() = z->value();
      if (mask)
        mask->z() = z->value() + 1;
    }
  }
  else if (ground_map)
  {
    double new_z = ground_map->z_at_point (pbody->value());
    abody->rescale (new_z);
    ahead->rescale (new_z);
    amouth->rescale (new_z);
    if (mask)
      mask->rescale (new_z + 1);
  }
}

bool Animation::compute_movement_from_path (C::Path_handle path)
{
  bool out = true;

  std::string id = path->entity();
  get<C::Boolean>(id , "walking")->set(true);
  auto abody = get<C::Animation>(id + "_body", "image");
  auto ahead = get<C::Animation>(id + "_head", "image");
  auto amouth = get<C::Animation>(id + "_mouth", "image");
  auto pbody = get<C::Position>(id + "_body", "position");
  auto phead = get<C::Position>(id + "_head", "position");
  auto pmouth = get<C::Position>(id + "_mouth", "position");

  Point pos = pbody->value();

  double to_walk = Config::character_speed * value<C::Double>(id, "speed_factor", 1.0);

  if (auto ground_map = get_ground_map(id))
    to_walk *= ground_map->z_at_point (pos) / Config::world_depth;

  Vector direction (pos, (*path)[path->current()]);
  direction.normalize();
  direction = to_walk * direction;
  direction = Vector (direction.x(), direction.y() / 3);

  while (true)
  {
    if (pos == (*path)[path->current()])
    {
      path->current() ++;
      if (path->current() == path->size())
      {
        out = false;
        generate_random_idle_animation(id, is_looking_right(id));
        break;
      }
      continue;
    }
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
      out = false;
      generate_random_idle_animation(id, current_vector.x() > 0);
      break;
    }
    path->current() ++;
  }

  pbody->set(pos);
  place_and_scale_character(id);

  return out;
}

void Animation::set_move_animation (const std::string& id, const Vector& direction)
{
  auto image = get<C::Animation>(id + "_body", "image");
  auto head = get<C::Animation>(id + "_head", "image");
  auto mouth = get<C::Animation>(id + "_mouth", "image");

  image->on() = true;

  if (head->on())
  {
    set<C::Double>(id, "walk_start_time", value<C::Double>(CLOCK__TIME));
    image->reset();
    head->reset();
    head->on() = false;
  }
  if (mouth->on())
  {
    mouth->reset();
    mouth->on() = false;
  }
  int row_index = 0;

  int angle = (int(180 * std::atan2 (direction.y(), direction.x()) / M_PI) + 360) % 360;

  const int limit = 33;
  if (angle < limit || angle >= (360 - limit))
    row_index = 1; // right
  else if (angle < (180 - limit))
    row_index = 0; // down
  else if (angle < (180 + limit))
    row_index = 2; // left
  else
    row_index = 3; // up

  if (auto speed_factor = request<C::Double> (id, "speed_factor"))
  {
    image->frames().clear();
    int nb_images = image->width_subdiv();
    int normal_id = 0;
    int fast_id = 0;
    int i = 0;
    do
    {
      image->frames().emplace_back();

      image->frames().back().x = int(fast_id);
      image->frames().back().y = row_index;
      image->frames().back().duration = 1;

      ++ i;
      normal_id = i % nb_images;
      fast_id = round(i * speed_factor->value()) % nb_images;
    }
    while (normal_id != 0 && fast_id != 0);
  }
  else
  {
    image->frames().resize(image->width_subdiv());
    for (std::size_t i = 0; i < std::size_t(image->width_subdiv()); ++ i)
    {
      image->frames()[i].x = int(i);
      image->frames()[i].y = row_index;
      image->frames()[i].duration = 1;
    }
  }

}

void Animation::generate_random_idle_animation (const std::string& id, bool looking_right)
{
  remove(id, "walk_start_time", true);

  // If character has no skin ("fake" character), do nothing
  if (!request<C::Group>(id, "group"))
    return;

  generate_random_idle_body_animation (id, looking_right);
  generate_random_idle_head_animation (id, looking_right);
}

void Animation::generate_random_idle_head_animation (const std::string& id, bool looking_right)
{
  debug << "Generate random idle head animation for character \"" << id << "\"" << std::endl;

  auto head = get<C::Animation>(id + "_head", "image");
  auto mouth = get<C::Animation>(id + "_mouth", "image");

  bool character_is_visible = get<C::Animation>(id + "_body", "image")->on();

  // Reset all
  head->reset();
  head->frames().clear();
  head->on() = character_is_visible;

  // Reset all
  mouth->reset();
  mouth->frames().clear();
  mouth->on() = character_is_visible;

  int row_index = 0;
  if (!looking_right)
    row_index = 1;

   mouth->frames().push_back ({0, row_index, 1});

  // Generate 10 poses with transitions
  int pose = 1;
  for (int i = 0; i < 10; ++ i)
  {
    int remaining = random_int(30, 300);

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
      head->frames().push_back ({pose, row_index, next_blink});

      if (remaining == 0)
        break;

      // Blink eyes
      head->frames().push_back ({0, row_index, 1});
    }

    if (random_int(0,4) == 0)
    {
      int new_pose = 1;
      if (head->width_subdiv() > 2)
      {
        do
        {
          new_pose = random_int(1, head->width_subdiv());
        }
        while (new_pose == pose);
      }
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
  debug << "Generate random idle body animation for character \"" << id << "\"" << std::endl;

  get<C::Boolean>(id , "walking")->set(false);
  auto image = get<C::Animation>(id + "_body", "image");

  // Reset all
  image->on() = true;
  image->reset();
  image->frames().clear();

  int row_index = 0;
  if (!looking_right)
    row_index = 1;


  std::vector<int> possibles_values;
  const std::vector<std::string>& positions
    = value<C::Vector<std::string> >(id + "_idle", "values");

  // Simple case: no animation
  if (positions[0] != "transition")
  {
    image->frames().push_back ({0, row_index, 1000});
    return;
  }

  int pose = 1;
  for (std::size_t i = 0; i < positions.size(); ++ i)
    if (positions[i] == "default")
    {
      possibles_values.push_back (int(i));
      pose = int(i);
    }
    else if (positions[i] == "idle")
      possibles_values.push_back (int(i));

  // Generate 10 poses with transitions

  for (int i = 0; i < 10; ++ i)
  {
    // Stand still for a while
    image->frames().push_back ({pose, row_index, random_int(50, 500)});

    // Transition
    image->frames().push_back ({0, row_index, 2});

    int new_pose;
    do
    {
      new_pose = possibles_values[std::size_t(random_int(0, int(possibles_values.size())))];
    }
    while (new_pose == pose);
    pose = new_pose;
  }
}

void Animation::generate_random_mouth_animation (const std::string& id)
{
  auto image = request<C::Animation>(id + "_mouth", "image");
  if (!image)
    return;

  // Reset all
  image->reset();
  image->frames().clear();
  image->on() = get<C::Animation>(id + "_body", "image")->on();

  int row_index = (get<C::Animation>(id + "_head", "image")->frames().front().y);

  // Generate 50 poses
  int pose = random_int(1, image->width_subdiv());
  for (int i = 0; i < 50; ++ i)
  {
    image->frames().push_back ({pose, row_index, 1});

    int new_pose;
    do
    {
      new_pose = random_int(1, image->width_subdiv());
    }
    while (new_pose == pose);
    pose = new_pose;

  }
}

void Animation::generate_animation (const std::string& id, const std::string& anim)
{
  debug << "Generate animation \"" << anim << "\" for character \"" << id << "\"" << std::endl;
  get<C::Boolean>(id , "walking")->set(false);
  auto image = get<C::Animation>(id + "_body", "image");
  const std::vector<std::string>& positions
    = value<C::Vector<std::string> >(id + "_idle", "values");

  int row_index = image->frames().front().y;

  // Reset all
  image->reset();
  image->frames().clear();


  int index = -1;
  for (std::size_t i = 0; i < positions.size(); ++ i)
    if (positions[i] == anim)
    {
      index = int(i);
      break;
    }
  check (index != -1, "No " + anim + " skin found for " + id);

  image->frames().push_back ({index, row_index, 1});
}

bool Animation::fade (double begin_time, double end_time, bool fadein)
{
  double current_time = value<C::Double>(CLOCK__TIME);

  double alpha = (fadein ? (end_time - current_time) / (end_time - begin_time)
                         : (current_time - begin_time) / (end_time - begin_time));

  if (alpha > 1)
    alpha = 1;
  if (alpha < 0)
    alpha = 0;

  auto img = get<C::Image>("Blackscreen", "image");
  img->on() = true;
  img->set_alpha((unsigned char)(255 * alpha));

  return (current_time < end_time);
}

void Animation::update_camera_target ()
{
  auto background = request<C::Image>("background", "image");
  if (!background)
    return;
  const std::string& id = value<C::String>("Player", "name");
  int xbody = value<C::Position>(id + "_body", "position").X();
  double xcamera = value<C::Absolute_position>(CAMERA__POSITION).x();

  double target = std::numeric_limits<double>::quiet_NaN();
  if (xbody < xcamera + Config::camera_limit_left)
    target = std::max (0, xbody - Config::camera_limit_right);
  else if (xbody > xcamera + Config::camera_limit_right)
  {
    int width = background->width();
    target = std::min (width - Config::world_width, xbody - Config::camera_limit_left);
  }

  if (std::isnan(target))
    return;

  if (request<C::GUI_animation>("Camera", "animation"))
    return;


  auto position = get<C::Position>(CAMERA__POSITION);
  if (target == position->value().X())
    return;

  double current_time = value<C::Double>(CLOCK__TIME);
  set<C::GUI_position_animation>("Camera", "animation", current_time, current_time + Config::camera_speed,
                                 position, Point (target, position->value().y()));
}


} // namespace Sosage::System
