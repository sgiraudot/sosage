/*
  [src/Sosage/System/Logic.cpp]
  Game logic, actions, dialog generation, etc.

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

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Code.h>
#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Cropped.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Logic.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Utils/geometry.h>

#include <algorithm>
#include <fstream>
#include <vector>

namespace Sosage::System
{

Logic::Logic (Content& content)
  : m_content (content), m_current_time(0)
  , m_current_action (nullptr), m_next_step (0)
{
}

void Logic::run ()
{
  auto status = m_content.get<Component::Status>(GAME__STATUS);
  if (status->value() == PAUSED || status->value() == LOADING)
    return;

  double current_time = m_content.get<Component::Double> (CLOCK__FRAME_TIME)->value();

  m_current_time = current_time;

  std::set<Timed_handle> new_timed_handle;

  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
    {
      const std::string& id = m_content.get<Component::String>("player:name")->value();
      auto saved_path = Component::cast<Component::Path>(th.second);
      auto current_path = m_content.request<Component::Path>(id + ":path");
      if (saved_path == current_path)
        new_timed_handle.insert(th);
    }
    else if (th.first <= m_current_time)
    {
      if (Component::cast<Component::Event>(th.second))
        m_content.set (th.second);
      else
        m_content.remove (th.second->id());
    }
    else
      new_timed_handle.insert(th);
  m_timed.swap(new_timed_handle);

  auto collision = m_content.request<Component::Image> ("cursor:target");
  auto clicked = m_content.request<Component::Event>("cursor:clicked");
  if (clicked && collision)
  {
    if (auto name = m_content.request<Component::String>(collision->entity() + ":name"))
    {
      if (m_content.get<Component::String> ("chosen_verb:text")->entity() == "verb_goto")
        compute_path_from_target(m_content.get<Component::Position>(collision->entity() + ":view"));
    }
    else
      compute_path_from_target(m_content.get<Component::Position>(CURSOR__POSITION));

    m_content.remove("cursor:clicked");
    m_content.remove("cursor:target");

    // Cancel current action
    clear_timed(true);
    m_current_action = nullptr;
  }

  if (auto code_clicked = m_content.request<Component::Event>("code:button_clicked"))
  {
    auto code = m_content.get<Component::Code>("game:code");
    auto window = m_content.get<Component::Image>("game:window");
    auto cropped
      = m_content.get<Component::Cropped>(window->entity() + "_button:image");

    cropped->crop (code->xmin(), code->xmax(), code->ymin(), code->ymax());
    m_content.set<Component::Position>
      (window->entity() + "_button:position",
       m_content.get<Component::Position>(window->entity() + ":position")->value()
       + Vector(code->xmin(), code->ymin()));

    cropped->on() = true;

    m_timed.insert (std::make_pair (m_current_time + Config::button_click_duration,
                                    Component::make_handle<Component::Event>
                                    ("code:stop_flashing")));

    if (code->failure())
    {
      code->reset();
      m_content.set<Component::Event>("code:play_failure");
    }
    else if (code->success())
    {
      code->reset();
      m_content.set<Component::Event>("code:play_success");
      m_timed.insert (std::make_pair (m_current_time + Config::button_click_duration,
                                      Component::make_handle<Component::Event>
                                      ("code:quit")));
    }
    else
      m_content.set<Component::Event>("code:play_click");

    m_content.remove("code:button_clicked");
  }

  if (auto stop_flashing = m_content.request<Component::Event>("code:stop_flashing"))
  {
    auto window = m_content.get<Component::Image>("game:window");
    auto cropped
      = m_content.get<Component::Cropped>(window->entity() + "_button:image");
    cropped->on() = false;
    m_content.remove("code:stop_flashing");
  }

  if (auto code_quit = m_content.request<Component::Event>("code:quit"))
  {
    auto code = m_content.get<Component::Code>("game:code");
    auto window = m_content.get<Component::Image>("game:window");
    window->on() = false;
    code->reset();
    status->pop();

    m_current_action = m_content.get<Component::Action>
      (m_content.get<Component::Code>("game:code")->entity() + ":action");
    m_next_step = 0;

    m_content.remove("code:quit");
  }

  auto action = m_content.request<Component::Action>("character:action");
  if (action && action != m_current_action)
  {
    clear_timed(false);
    m_current_action = action;
    m_next_step = 0;
    m_content.remove ("character:action", true);
  }

  if (m_current_action)
  {
    if (m_timed.empty())
    {
      if (m_next_step == m_current_action->size())
      {
        m_current_action = nullptr;
      }
      else
        while (m_next_step != m_current_action->size())
        {
          const Component::Action::Step& s = (*m_current_action)[m_next_step ++];

          if (s.get(0) == "comment")
            action_comment (s);
          else if (s.get(0) == "goto")
            action_goto (m_current_action->entity());
          else if (s.get(0) == "load")
            action_load (s);
          else if (s.get(0) == "look")
            action_look (m_current_action->entity());
          else if (s.get(0) == "move")
            action_move (s);
          else if (s.get(0) == "play")
            action_play (s);
          else if (s.get(0) == "animate")
            action_animate (s);
          else if (s.get(0) == "set_state")
            action_set_state (s);
          else if (s.get(0) == "set_coordinates")
            action_set_coordinates (s);
          else if (s.get(0) == "lock")
            status->push(LOCKED);
          else if (s.get(0) == "unlock")
            status->pop();
          else if (s.get(0) == "show")
            action_show (s);
          else if (s.get(0) == "sync")
            break;
        }
    }
  }

  update_camera();
  update_debug_info (m_content.get<Component::Debug>(GAME__DEBUG));
}

void Logic::clear_timed(bool action_goto)
{
  std::set<Timed_handle> new_timed_handle;

  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
      continue;
    else if (action_goto && th.second->id().find("comment_") == 0) // keep dialogs when moving
    {
      new_timed_handle.insert(th);
    }
    else
    {
      if (Component::cast<Component::Event>(th.second))
        m_content.set (th.second);
      else
        m_content.remove (th.second->id());
    }
  m_timed.swap(new_timed_handle);
}

bool Logic::compute_path_from_target (Component::Position_handle target)
{
  auto ground_map = m_content.get<Component::Ground_map>("background:ground_map");

  const std::string& id = m_content.get<Component::String>("player:name")->value();
  auto position = m_content.get<Component::Position>(id + "_body:position");

  Point origin = position->value();
  Point t = target->value();

  if (target->component() != "view")
    t = t + Vector (m_content.get<Component::Double>(CAMERA__POSITION)->value(), 0);

  std::vector<Point> path;
  ground_map->find_path (origin, t, path);

  // Check if character is already at target
  if ((path.size() == 1) && (path[0] == origin))
    return false;

  m_content.set<Component::Path>(id + ":path", path);
  return true;
}

void Logic::update_camera()
{
  auto position = m_content.get<Component::Double>(CAMERA__POSITION);
  auto target = m_content.get<Component::Double>("camera:target");

  double dir = target->value() - position->value();
  dir *= Config::camera_speed;
  position->set (position->value() + dir);
}

void Logic::update_debug_info (Component::Debug_handle debug_info)
{
  if (debug_info->value())
  {
    auto debug_font = m_content.get<Component::Font> ("debug:font");
    auto dbg_img = m_content.set<Component::Image> ("debug:image",
                                                    debug_font, "FF0000",
                                                    debug_info->debug_str());
    auto dbg_pos = m_content.set<Component::Position>("debug:position", Point(0,0));
  }
  else
  {
    auto dbg_img = m_content.request<Component::Image> ("debug:image");
    if (dbg_img)
      dbg_img->on() = false;

  }

}

void Logic::action_comment (Component::Action::Step step)
{
  const std::string& id = m_content.get<Component::String>("player:name")->value();

  std::string text = step.get(1);

  std::vector<Component::Image_handle> dialog;
  create_dialog (text, dialog);

  int nb_char = int(text.size());
  double nb_seconds = nb_char / m_content.get<Component::Double>("text:char_per_second")->value();

  int y = 100;
  int x = m_content.get<Component::Position>(id + "_body:position")->value().x()
          - m_content.get<Component::Double>(CAMERA__POSITION)->value();

  for (auto img : dialog)
    if (x + 0.75 * img->width() / 2 > int(0.95 * Config::world_width))
      x = int(0.95 * Config::world_width) - 0.75 * img->width() / 2;
    else if (x - 0.75 * img->width() / 2 < int(0.1 * Config::world_width))
      x = int(0.1 * Config::world_width) + 0.75 * img->width() / 2;


  for (auto img : dialog)
  {
    auto pos = m_content.set<Component::Position> (img->entity() + ":position", Point(x,y));
    y += img->height() * 1.1 * 0.75;

    m_timed.insert (std::make_pair (m_current_time + std::max(1., nb_seconds), img));
    m_timed.insert (std::make_pair (m_current_time + std::max(1., nb_seconds), pos));
  }

  m_content.set<Component::Event>(id + ":start_talking");

  m_timed.insert (std::make_pair (m_current_time + nb_seconds,
                                  Component::make_handle<Component::Event>
                                  (id + ":stop_talking")));
}

void Logic::action_goto (const std::string& target)
{
  const std::string& id = m_content.get<Component::String>("player:name")->value();

  auto position = m_content.request<Component::Position>(target + ":view");
  if (compute_path_from_target(position))
    m_timed.insert (std::make_pair (0, m_content.get<Component::Path>(id + ":path")));
}

void Logic::action_load (Component::Action::Step step)
{
  m_content.set<Component::String>("game:new_room", step.get(1));
  m_content.set<Component::String>("game:new_room_origin", step.get(2));
}

void Logic::action_look (const std::string& target)
{
  const std::string& id = m_content.get<Component::String>("player:name")->value();

  if (target == "default" || !m_content.request<Component::Position>(target + ":position"))
    m_content.set<Component::Position>(id + ":lookat",
                                       m_content.get<Component::Position>(CURSOR__POSITION)->value());
  else
  {
    auto state = m_content.request<Component::String>(target + ":state");
    if (!state || state->value() != "inventory")
      m_content.set<Component::Position>(id + ":lookat",
                                         m_content.get<Component::Position>(target + ":position")->value());
  }
}

void Logic::action_move (Component::Action::Step step)
{
  std::string target = step.get(1);
  int x = step.get_int(2);
  int y = step.get_int(3);
  int z = step.get_int(4);

  m_content.set<Component::Position>(target + ":position", Point(x, y));
  m_content.get<Component::Image>(target + ":image")->z() = z;
}

void Logic::action_play (Component::Action::Step step)
{
  std::string target = step.get(1);

  auto animation = m_content.get<Component::Animation>(target + ":image");
  animation->reset (true, 1);
  animation->on() = true;
}

void Logic::action_animate (Component::Action::Step step)
{
  const std::string& character = m_content.get<Component::String>("player:name")->value();

  std::string id = step.get(1);
  double duration = step.get_double(2);
  m_content.set<Component::String>(character + ":start_animation", id);

  m_timed.insert (std::make_pair (m_current_time + duration,
                                  Component::make_handle<Component::Event>
                                  (character + ":stop_animation")));

}

void Logic::action_set_state (Component::Action::Step step)
{
  std::string target = step.get(1);

  auto current_state = m_content.get<Component::String>(target + ":state");

  std::string state;
  if (step.size() == 3)
    state = step.get(2);
  else // if (step.size() == 4)
  {
    if (step.get(2) != current_state->value())
      return;
    state = step.get(3);
  }

  if (current_state->value() == "inventory")
  {
    m_content.get<Component::Inventory>("game:inventory")->remove(target);
    m_content.get<Component::Position>(target + ":position")->absolute() = false;
  }

  current_state->set (state);
  if (state == "inventory")
  {
    m_content.get<Component::Inventory>("game:inventory")->add(target);
    auto img
      = m_content.get<Component::Image>(target + ":image");
    img->set_relative_origin(0.5, 0.5);
    img->z() = Config::inventory_back_depth;
  }
}

void Logic::action_set_coordinates (Component::Action::Step step)
{
  std::string target = step.get(1);
  int x = step.get_int(2);
  int y = step.get_int(3);
  int z = step.get_int(4);

  m_content.get<Component::Position>(target + ":position")->set (Point(x, y));
  m_content.get<Component::Image>(target + ":image")->z() = z;
}

void Logic::action_show (Component::Action::Step step)
{
  std::string target = step.get(1);

  auto image = m_content.get<Component::Image>(target + ":image");
  image->on() = true;

  m_content.set<Component::Variable>("game:window", image);

  auto code = m_content.request<Component::Code>(target + ":code");
  if (code)
  {
    m_content.get<Component::Status>(GAME__STATUS)->push (IN_CODE);
    m_content.set<Component::Variable>("game:code", code);
  }
  else
    m_content.get<Component::Status>(GAME__STATUS)->push (IN_WINDOW);
}

void Logic::create_dialog (const std::string& text, std::vector<Component::Image_handle>& dialog)
{
  static const int width_max = int(0.95 * Config::world_width);

  auto interface_font = m_content.get<Component::Font> ("interface:font");
  const std::string& color
      = m_content.get<Component::String> (m_content.get<Component::String>("player:name")->value()
                                          + ":color")->value();

  auto img
    = m_content.set<Component::Image> ("comment:image",
                                       interface_font,
                                       color,
                                       text, true);
  img->set_scale(0.75);
  img->set_relative_origin(0.5, 0.5);

  if (img->width() <= width_max)
    dialog.push_back (img);
  else
  {
    m_content.remove("comment:image");
    int nb_imgs = 1 + (img->width() / width_max);

    // Find space characters where to cut
    std::vector<std::size_t> cuts (nb_imgs  - 1);

    for (std::size_t i = 0; i < cuts.size(); ++ i)
    {
      std::size_t cut = std::size_t((i+1) * (text.size() / double(nb_imgs)));
      if (text[cut] == ' ')
        cuts[i] = cut;
      else
      {
        std::size_t candidate_before = cut;
        while (candidate_before != 0 && text[candidate_before] != ' ')
          -- candidate_before;
        if (text[candidate_before] != ' ')
          candidate_before = 10000;

        std::size_t candidate_after = cut;
        while (candidate_after != text.size() && text[candidate_after] != ' ')
          ++ candidate_after;
        if (text[candidate_after] != ' ')
          candidate_after = 10000;

        if (candidate_after - cut < cut - candidate_before)
          cuts[i] = candidate_after;
        else
          cuts[i] = candidate_before;
        dbg_check (text[cuts[i]] == ' ', "Cutting dialog went wrong");
      }
    }

    for (int i = 0; i < nb_imgs; ++ i)
    {
      std::size_t begin = (i == 0 ? 0 : cuts[i-1] + 1);
      std::size_t end = (i == nb_imgs - 1 ? text.size() : cuts[i]);
      auto img
        = m_content.set<Component::Image> ("comment_" + std::to_string(i) + ":image",
                                           interface_font,
                                           color,
                                           std::string(text.begin() + begin,
                                                       text.begin() + end), true);
      img->z() = Config::inventory_over_depth;
      img->set_scale(0.75);
      img->set_relative_origin(0.5, 0.5);
      dialog.push_back (img);
    }
  }

}


} // namespace Sosage::System
