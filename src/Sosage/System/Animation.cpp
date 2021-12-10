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

namespace Sosage::System
{

namespace C = ::Sosage::Component;

Animation::Animation (Content& content)
  : Base (content), m_frame_id(0), m_fade_to_remove(false)
{
}

void Animation::run()
{
  start_timer();
  std::size_t new_frame_id = frame_id(value<C::Double>(CLOCK__TIME));

  if (status()->is (PAUSED))
  {
    m_frame_id = new_frame_id;
    stop_timer("Animation");
    return;
  }

  run_gui_frame();

  if (status()->is (IN_MENU))
  {
    m_frame_id = new_frame_id;
    stop_timer("Animation");
    return;
  }

  if (new_frame_id == m_frame_id)
  {
    // Force update when new room is loaded
    if (request<C::Boolean>("Game:in_new_room"))
      run_animation_frame();
    stop_timer("Animation");
    return;
  }

  for (std::size_t i = m_frame_id; i < new_frame_id; ++ i)
    run_animation_frame();
  m_frame_id = new_frame_id;
  stop_timer("Animation");
}

void Animation::run_gui_frame()
{
  if (auto fadein = request<C::Boolean>("Fade:in"))
  {
    fade (value<C::Double>("Fade:begin"), value<C::Double>("Fade:end"), fadein->value());
    m_fade_to_remove = fadein->value();
  }
  else if (m_fade_to_remove)
  {
    get<C::Image>("Blackscreen:image")->on() = false;
    m_fade_to_remove = false;
  }

  double current_time = value<C::Double>(CLOCK__TIME);
  if (auto i = request<C::Double>("Shake:intensity"))
  {
    double begin = value<C::Double>("Shake:begin");
    double end = value<C::Double>("Shake:end");
    double intensity = i->value();
    double x_start = value<C::Double>("Camera:saved_position");

    double current_intensity = intensity * (end - current_time) / (end - begin);

    constexpr double period = 0.02;

    double shift = std::sin ((current_time - begin) / period);

    get<C::Absolute_position>(CAMERA__POSITION)->set
        (Point(x_start + shift * current_intensity, 0));
  }

  std::vector<C::GUI_animation_handle> to_remove;
  for (auto c : components("animation"))
    if (auto a = C::cast<C::GUI_animation>(c))
      if (!a->update(current_time))
        to_remove.emplace_back(a);

  for (C::GUI_animation_handle a : to_remove)
  {
    if (a->remove_after())
    {
      const std::string& object_id = a->object_id();
      remove(object_id);
      std::size_t pos = object_id.find(":image");
      if (pos != std::string::npos)
        remove(std::string(object_id.begin(), object_id.begin() + pos) + ":position");
    }
    remove(a->id());
  }
}

void Animation::run_animation_frame()
{
  if (auto new_char = request<C::Vector<std::pair<std::string, bool> > >("Game:new_characters"))
  {
    for (const auto& nc : new_char->value())
    {
      place_and_scale_character (nc.first, nc.second);
      generate_random_idle_animation (nc.first, nc.second);
    }
    remove("Game:new_characters");
  }

  if (auto looking_right = request<C::Boolean>("Game:in_new_room"))
  {
    const std::string& player = value<C::String>("Player:name");
    place_and_scale_character (player, looking_right->value());
    generate_random_idle_animation (player, looking_right->value());

    // Relaunch animations
    for (auto c : components("image"))
      if (request<C::String>(c->entity() + ":state"))
        if (auto anim = C::cast<C::Animation>(c))
          anim->on() = true;

    remove("Game:in_new_room");
  }

  std::vector<std::string> to_remove;
  std::vector<C::Animation_handle> animations;

  // First check if some character should change looking direction
  for (auto c : components("lookat"))
  {
    debug << "lookat" << std::endl;
    const std::string& id = c->entity();
    auto lookat = C::cast<C::Position>(c);
    auto abody = get<C::Animation>(id + "_body:image");
    auto ahead = get<C::Animation>(id + "_head:image");
    auto pbody = get<C::Position>(id + "_body:position");
    auto phead = get<C::Position>(id + "_head:position");
    auto pmouth = get<C::Position>(id + "_mouth:position");

    Vector direction (pbody->value(), lookat->value());
    bool looking_right = (direction.x() > 0);

    if (looking_right)
    {
      phead->set (pbody->value() - abody->core().scaling
                  * Vector(value<C::Position>(id + "_head:gap_right")));
      pmouth->set (phead->value() - ahead->core().scaling
                   * Vector(value<C::Position>(id + "_mouth:gap_right")));
    }
    else
    {
      phead->set (pbody->value() - abody->core().scaling
                  * Vector(value<C::Position>(id + "_head:gap_left")));
      pmouth->set (phead->value() - ahead->core().scaling
                   * Vector(value<C::Position>(id + "_mouth:gap_left")));
    }

    generate_random_idle_animation (id, looking_right);
    to_remove.push_back (c->id());
  }

  // Then check animations stopping
  for (auto c : components("stop_talking"))
  {
    const std::string& id = c->entity();
    generate_random_idle_head_animation (id,
                                         get<C::Animation>(id + "_head:image")
                                         ->frames().front().y == 0);
    to_remove.push_back (c->id());
  }

  for (auto c : components("stop_walking"))
  {
    const std::string& id = c->entity();
    if (value<C::Boolean>(id + ":walking"))
    {
      bool looking_right = (get<C::Animation>(id + "_body:image")->frames().front().y != 2);
      generate_random_idle_animation (id, looking_right);
      place_and_scale_character(id, looking_right);
    }
    to_remove.push_back (c->id());
  }

  for (auto c : components("stop_animation"))
  {
    const std::string& id = c->entity();
    debug << "stop_animation" << std::endl;
    if (auto head = request<C::Animation>(id + "_head:image"))
      generate_random_idle_body_animation (id, head->frames().front().y == 0);
    else
      get<C::Image>(id + ":image")->on() = false;
    to_remove.push_back (c->id());
  }

  bool has_moved = false;

  std::unordered_set<std::string> just_started;

  // Then check all other cases
  for (auto c : components("move"))
    if (auto a = C::cast<C::Tuple<Point, Point, double, double>>(c))
    {
      double ftime = frame_time(value<C::Double>(CLOCK__TIME));
      double ratio = (ftime - a->get<2>()) / (a->get<3>() - a->get<2>());

      Point current = ratio * a->get<1>() + (1 - ratio) * a->get<0>();
      if (ratio > 1)
        current = a->get<1>();

      get<C::Position>(a->entity() + ":position")->set (current);
    }

  for (auto c : components("rescale"))
    if (auto a = C::cast<C::Tuple<double, double, double, double>>(c))
    {
      double ftime = frame_time(value<C::Double>(CLOCK__TIME));
      double ratio = (ftime - a->get<2>()) / (a->get<3>() - a->get<2>());

      double scale = ratio * a->get<1>() + (1 - ratio) * a->get<0>();
      if (ratio > 1)
        scale = a->get<1>();

      get<C::Image>(a->entity() + ":image")->set_scale (scale);
    }


  for (auto c : components("path"))
    if (auto path = C::cast<C::Path>(c))
    {
      if (!compute_movement_from_path(path))
        to_remove.push_back(c->id());
      else if (path->entity() == value<C::String>("Player:name"))
        has_moved = true;
    }

  for (auto c : components("start_talking"))
  {
    const std::string& id = c->entity();
    generate_random_mouth_animation (id);
    to_remove.push_back(c->id());
  }

  for (auto c : components("start_animation"))
  {
    const std::string& id = c->entity();
    if (auto s = C::cast<C::Signal>(c))
    {
      get<C::Animation>(id + ":image")->on() = true;
      just_started.insert (id);
      to_remove.push_back(c->id());
    }
    else
    {
      auto anim = C::cast<C::String>(c);
      const std::string& id = c->entity();
      debug << "start_animation" << std::endl;
      generate_animation (id, anim->value());
      to_remove.push_back (c->id());
    }
  }

  for (auto c : components("set_visible"))
  {
    debug << "Set " << c->id() << " visible" << std::endl;
    const std::string& id = c->entity();
    auto g = get<C::Group>(id + ":group");
    g->apply<C::Image>([](auto img) { img->on() = true; });
    to_remove.push_back(c->id());
  }

  for (auto c : components("set_hidden"))
  {
    debug << "Set " << c->id() << " hidden" << std::endl;
    const std::string& id = c->entity();
    auto g = get<C::Group>(id + ":group");
    g->apply<C::Image>([](auto img) { img->on() = false; });
    to_remove.push_back(c->id());
  }

  for (auto c : components("image"))
    if (auto anim = C::cast<C::Animation>(c))
      if (anim->on())
        animations.push_back(anim);

  for (const std::string& c : to_remove)
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
  m_content.get<Component::Animation>("Loading_spin:image")->next_frame();
  return true;
}

void Animation::place_and_scale_character(const std::string& id, bool looking_right)
{
  auto abody = get<C::Animation>(id + "_body:image");
  auto ahead = get<C::Animation>(id + "_head:image");
  auto amouth = get<C::Animation>(id + "_mouth:image");
  auto pbody = get<C::Position>(id + "_body:position");
  auto phead = get<C::Position>(id + "_head:position");
  auto pmouth = get<C::Position>(id + "_mouth:position");

  double new_z;
  if (auto ground_map = request<C::Ground_map>("Background:ground_map"))
    new_z = ground_map->z_at_point (pbody->value());
  else
    new_z = value<C::Int>("Background:default_z");
  abody->rescale (new_z);
  ahead->rescale (new_z);
  ahead->z() += 1;
  amouth->rescale (new_z);
  amouth->z() += 2;

  if (looking_right)
  {
    phead->set (pbody->value() - abody->core().scaling
                * Vector(value<C::Position>(id + "_head:gap_right")));
    pmouth->set (phead->value() - ahead->core().scaling
                * Vector(value<C::Position>(id + "_mouth:gap_right")));
  }
  else
  {
    phead->set (pbody->value() - abody->core().scaling
                * Vector(value<C::Position>(id + "_head:gap_left")));
    pmouth->set (phead->value() - ahead->core().scaling
                * Vector(value<C::Position>(id + "_mouth:gap_left")));
  }
}

bool Animation::compute_movement_from_path (C::Path_handle path)
{
  bool out = true;

  std::string id = path->entity();
  get<C::Boolean>(id + ":walking")->set(true);
  auto abody = get<C::Animation>(id + "_body:image");
  auto ahead = get<C::Animation>(id + "_head:image");
  auto amouth = get<C::Animation>(id + "_mouth:image");
  auto pbody = get<C::Position>(id + "_body:position");
  auto phead = get<C::Position>(id + "_head:position");
  auto pmouth = get<C::Position>(id + "_mouth:position");
  auto ground_map = get<C::Ground_map>("Background:ground_map");

  Point pos = pbody->value();

  double to_walk = Config::character_speed;

  if (auto ground_map = request<C::Ground_map>("Background:ground_map"))
    to_walk *= ground_map->z_at_point (pos) / Config::world_depth;
  else
    to_walk *= value<C::Int>("Background:default_z");

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
        generate_random_idle_animation(id, get<C::Animation>(id + "_head:image")
                                       ->frames().front().y == 0);
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
  place_and_scale_character(id, direction.x() > 0);

  return out;
}

void Animation::set_move_animation (const std::string& id, const Vector& direction)
{
  auto image = get<C::Animation>(id + "_body:image");
  auto head = get<C::Animation>(id + "_head:image");
  auto mouth = get<C::Animation>(id + "_mouth:image");

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

  image->frames().resize(8);
  for (std::size_t i = 0; i < 8; ++ i)
  {
    image->frames()[i].x = int(i);
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
  debug << "Generate random idle head animation for character \"" << id << "\"" << std::endl;

  auto head = get<C::Animation>(id + "_head:image");
  auto mouth = get<C::Animation>(id + "_mouth:image");

  // Reset all
  head->reset();
  head->frames().clear();
  head->on() = true;

  // Reset all
  mouth->reset();
  mouth->frames().clear();
  mouth->on() = true;

  int row_index = 0;
  if (!looking_right)
    row_index = 1;

  // Generate 10 poses with transitions
  int pose = 0;
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
      mouth->frames().push_back ({0, row_index, next_blink});

      if (remaining == 0)
        break;

      // Blink eyes
      head->frames().push_back ({1, row_index, 1});
      mouth->frames().push_back ({0, row_index, 1});
    }

    if (random_int(0,4) == 0)
    {
      int new_pose;
      do
      {
        new_pose = random_int(2, head->width_subdiv());
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
  debug << "Generate random idle body animation for character \"" << id << "\"" << std::endl;

  get<C::Boolean>(id + ":walking")->set(false);
  auto image = get<C::Animation>(id + "_body:image");

  // Reset all
  image->on() = true;
  image->reset();
  image->frames().clear();

  int row_index = 0;
  if (!looking_right)
    row_index = 1;

  std::vector<int> possibles_values;
  const std::vector<std::string>& positions
    = value<C::Vector<std::string> >(id + "_idle:values");

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
  auto image = get<C::Animation>(id + "_mouth:image");
  // Reset all
  image->reset();
  image->frames().clear();
  image->on() = true;

  int row_index = (get<C::Animation>(id + "_head:image")->frames().front().y);

  // Generate 50 poses
  int pose = random_int(1,11);
  for (int i = 0; i < 50; ++ i)
  {
    image->frames().push_back ({pose, row_index, 1});

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
  debug << "Generate animation \"" << anim << "\" for character \"" << id << "\"" << std::endl;
  get<C::Boolean>(id + ":walking")->set(false);
  auto image = get<C::Animation>(id + "_body:image");
  const std::vector<std::string>& positions
    = value<C::Vector<std::string> >(id + "_idle:values");

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

void Animation::fade (double begin_time, double end_time, bool fadein)
{
  double current_time = value<C::Double>(CLOCK__TIME);

  if (current_time > end_time)
    return;

  double alpha = (fadein ? (end_time - current_time) / (end_time - begin_time)
                         : (current_time - begin_time) / (end_time - begin_time));

  if (alpha > 1)
    alpha = 1;
  if (alpha < 0)
    alpha = 0;

  auto img = get<C::Image>("Blackscreen:image");
  img->on() = true;
  img->set_alpha((unsigned char)(255 * alpha));
}

void Animation::update_camera_target ()
{
  const std::string& id = value<C::String>("Player:name");
  int xbody = value<C::Position>(id + "_body:position").X();
  double xcamera = value<C::Absolute_position>(CAMERA__POSITION).x();

  double target = std::numeric_limits<double>::quiet_NaN();
  if (xbody < xcamera + Config::camera_limit_left)
    target = std::max (0, xbody - Config::camera_limit_right);
  else if (xbody > xcamera + Config::camera_limit_right)
  {
    int width = get<C::Image>("Background:image")->width();
    target = std::min (width - Config::world_width, xbody - Config::camera_limit_left);
  }

  if (std::isnan(target))
    return;

  if (request<C::GUI_animation>("Camera:animation"))
    return;

  double current_time = value<C::Double>(CLOCK__TIME);
  auto position = get<C::Position>(CAMERA__POSITION);
  set<C::GUI_position_animation>("Camera:animation", current_time, current_time + Config::camera_speed,
                                 position, Point (target, position->value().y()));
}

} // namespace Sosage::System
