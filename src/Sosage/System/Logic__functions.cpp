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
#include <Sosage/Component/Cutscene.h>
#include <Sosage/Component/Dialog.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Group.h>
#include <Sosage/Component/GUI_animation.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Music.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Simple.h>
#include <Sosage/Component/Status.h>
#include <Sosage/Component/Variable.h>
#include <Sosage/System/Logic.h>
#include <Sosage/Utils/conversions.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Utils/geometry.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <vector>

namespace Sosage::System
{

namespace C = Component;

bool Logic::function_add (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_add takes 2 arguments");
  std::string id = args[0];

  auto integer = request<C::Int>(id + ":value");
  if (integer)
  {
    int diff = to_int(args[1]);
    integer->set (integer->value() + diff);

    auto action = request<C::Action>(id + ":" + std::to_string(integer->value()));
    if (!action)
      action = get<C::Action>(id + ":default");

    action->launch();
    while (action->on())
      apply_next_step (action);
  }
  else
  {
    auto list = request<C::Vector<std::string>>(id + ":list");
    if (!list)
      list = set<C::Vector<std::string>>(id + ":list");
    list->push_back (args[1]);
  }
  return true;
}

bool Logic::function_camera (const std::vector<std::string>& args)
{
  check (1 <= args.size() && args.size() <= 3, "function_camera takes 1, 2 or 3 arguments");
  int xtarget = to_int(args[0]);
  int ytarget = (args.size() > 1 ? to_int(args[1]) : 0);
  auto position = get<C::Position>(CAMERA__POSITION);

  if (args.size() == 3)
  {
    double zoom = to_double(args[2]);
    position->set (Point (xtarget, ytarget));
    get<C::Double>(CAMERA__ZOOM)->set(zoom);
  }
  else
    set<C::GUI_position_animation>("Camera:animation", m_current_time, m_current_time + Config::camera_speed,
                                   position, Point (xtarget, ytarget));
  return true;
}

bool Logic::function_control (const std::vector<std::string>& args)
{
  check (args.size() == 1 or args.size() == 2, "function_load control takes 1 or 2 arguments");
  std::string leader = args[0];
  set<C::String>("Player:name", leader);

  if (args.size() == 2)
  {
    std::string follower = args[1];
    set<C::String>("Follower:name", follower);
  }
  else
    remove ("Follower:name");
  return true;
}

bool Logic::function_exit (const std::vector<std::string>&)
{
  emit ("Game:exit");
  return true;
}

bool Logic::function_fadein (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_fadein takes 1 argument");
  return subfunction_fade (true, to_double(args[0]));
}

bool Logic::function_fadeout (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_fadein takes 1 argument");
  return subfunction_fade (false, to_double(args[0]));
}

bool Logic::function_goto (const std::vector<std::string>& init_args)
{
  check (init_args.size() <= 3, "function_goto takes at most 3 arguments");
  std::string id = "";
  std::vector<std::string> args;
  if (init_args.size() > 0 && request<C::Image>(init_args[0] + "_head:image"))
  {
    id = init_args[0];
    args = std::vector<std::string>(init_args.begin() + 1, init_args.end());
  }
  else
  {
    id = value<C::String>("Player:name");
    args = init_args;
  }

  if (args.size() == 2)
  {
    if (compute_path_from_target
        (C::make_handle<C::Absolute_position>("Goto:view", Point (to_int(args[0]), to_int(args[1]))),
         id))
     m_current_action->schedule (0, get<C::Path>(id + ":path"));
  }
  else
  {
    std::string target
        = (args.empty() ? get<C::Action>("Character:action")->target_entity() : args[0]);
    debug << "Action_goto " << target << std::endl;

    auto position = request<C::Position>(target + ":view");
    if (compute_path_from_target(position, id))
     m_current_action->schedule (0, get<C::Path>(id + ":path"));
  }

  return true;
}

bool Logic::function_hide (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_hide takes 1 argument");
  std::string target = args[0];
  if (auto question = request<C::String>(target + ":question"))
    get<C::Set<std::string>>("Hints:list")->erase(target);
  else
    emit (target + ":set_hidden");
  return true;
}

bool Logic::function_load (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_load takes 2 arguments");
  set<C::String>("Game:new_room", args[0]);
  set<C::String>("Game:new_room_origin", args[1]);
  return true;
}

bool Logic::function_lock (const std::vector<std::string>&)
{
  status()->push(LOCKED);
  return true;
}

bool Logic::function_look (const std::vector<std::string>& args)
{
  check (args.size() <= 2, "function_look takes at most 2 argument");

  std::string target = "";
  std::string id = "";
  if (args.empty())
  {
    id = value<C::String>("Player:name");
    target = get<C::Action>("Character:action")->target_entity();
  }
  else if (args.size() == 1)
  {
    id = value<C::String>("Player:name");
    target = args[0];
  }
  else // if (args.size() == 2)
  {
    id = args[0];
    target = args[1];
  }

  debug << "Action_look " << id << " " << target << std::endl;

  if (target == "default" || !request<C::Position>(target + ":position"))
    set<C::Absolute_position>(id + ":lookat",
                              value<C::Position>(CURSOR__POSITION));
  else
  {
    auto state = request<C::String>(target + ":state");
    if (!state || state->value() != "inventory")
      set<C::Absolute_position>(id + ":lookat",
                                value<C::Position>(target + ":position"));
  }
  return true;
}

bool Logic::function_move (const std::vector<std::string>& args)
{
  check (args.size() == 4 || args.size() == 5, "function_move takes 4 or 5 arguments");
  std::string target = args[0];
  int x = to_int(args[1]);
  int y = to_int(args[2]);
  int z = to_int(args[3]);
  if (args.size() == 5) // Smooth move
  {
    double duration = to_double(args[4]);
    int nb_frames = round (duration * Config::animation_fps);

    double begin_time = frame_time(m_current_time);
    double end_time = begin_time + (nb_frames + 0.5) / double(Config::animation_fps);

   m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy:event"));

    auto pos = get<C::Position>(target + ":position");

    auto anim = set<C::Tuple<Point, Point, double, double>>
        (target + ":animation", pos->value(), Point(x,y), begin_time, end_time);
   m_current_action->schedule (end_time,  anim);
  }
  else
  {
    get<C::Position>(target + ":position")->set (Point(x, y));
    get<C::Image>(target + ":image")->z() = z;
  }
  return true;
}

bool Logic::function_play (const std::vector<std::string>& args)
{
  check (args.size() == 2 || args.size() == 1, "function_play takes 1 or 2 arguments");
  std::string target = args[0];

  if (request<C::Handle>(target + ":sound"))
  {
    emit (target + ":play_sound");
    return true;
  }
  if (auto music = request<C::Music>(target + ":music"))
  {
    set<C::Variable>("Game:music", music);
    emit ("Music:start");
    return true;
  }

  // else, animation
  if (args.size() == 2) // Target is character
  {
    const std::string& character = value<C::String>("Player:name");

    double duration = to_double(args[1]);
    set<C::String>(character + ":start_animation", target);

    if (duration > 0)
      m_current_action->schedule (m_current_time + duration,
                                  C::make_handle<C::Signal>
                                  (character + ":stop_animation"));
  }
  else
  {
    auto animation = get<C::Animation>(target + ":image");
    emit (target + ":start_animation");

    if (animation->loop())
    {
      // If animation loop, add a fake state so that it's reloaded
      // on if we exist and reener the room
      set<C::String>(target + ":state", "Dummy");
    }
    else
    {
      // If animation does not loop, insert dummy timed Event
      // so that sync waits for the end of the animation
      int nb_frames = 0;
      for (const auto& f : animation->frames())
        nb_frames += f.duration;
      double latest_frame_time = frame_time(m_current_time);
      double end_time = latest_frame_time + (nb_frames + 0.5) / double(Config::animation_fps);

      m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy:event"));
    }
  }
  return true;
}

bool Logic::function_rescale (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_rescale takes 2 arguments");
  std::string target = args[0];
  double scale = to_double(args[1]);
  get<C::Image>(target + ":image")->set_scale(scale);
  return true;
}


bool Logic::function_set (const std::vector<std::string>& args)
{
  check (args.size() == 2 || args.size() == 3, "function_set takes 2 or 3 arguments");
  std::string target = args[0];
  auto current_state = get<C::String>(target + ":state");
  std::string state = args[1];

  if (args.size() == 3)
  {
    if (state != current_state->value())
      return true;
    state = args[2];
  }

  if (current_state->value() == "inventory")
  {
    get<C::Inventory>("Game:inventory")->remove(target);
    //get<C::Absolute_position>(target + ":position")->absolute() = false;
  }

  current_state->set (state);
  if (state == "inventory")
  {
    get<C::Inventory>("Game:inventory")->add(target);
    auto img
        = get<C::Image>(target + ":image");
    img->set_relative_origin(0.5, 0.5);
    img->z() = Config::inventory_depth;
    img->on() = false;
  }

  // Changing the state an object might change the labels of its
  // related action, force an update if using gamepad
  if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD)
    emit("Action_selector:force_update");

  return true;
}

bool Logic::function_shake (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_shake takes 2 arguments");
  double intensity  = to_double(args[0]);
  double duration = to_double(args[1]);
  auto begin = set<C::Double> ("Shake:begin", m_current_time);
  auto end = set<C::Double> ("Shake:end", m_current_time + duration);
  auto intens = set<C::Double> ("Shake:intensity", intensity);
  auto camera = set<C::Double> ("Camera:saved_position", value<C::Absolute_position>(CAMERA__POSITION).x());
  m_current_action->schedule (m_current_time + duration, begin);
  m_current_action->schedule (m_current_time + duration, end);
  m_current_action->schedule (m_current_time + duration, intens);
  m_current_action->schedule (m_current_time + duration, camera);
  return true;
}

bool Logic::function_show (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_show takes 2 arguments");
  std::string target = args[0];
  if (auto group = request<C::Group>(target + ":group")) // Character
    emit (target + ":set_visible");
  else if (auto question = request<C::String>(target + ":question"))
    get<C::Set<std::string>>("Hints:list")->insert (target);
  else
  {
    auto image = get<C::Image>(target + ":image");
    set<C::Variable>("Game:window", image);
    emit ("Interface:show_window");

    auto code = request<C::Code>(target + ":code");
    if (code)
    {
      status()->push (IN_CODE);
      set<C::Variable>("Game:code", code);
    }
    else
      status()->push (IN_WINDOW);
  }
  return true;
}

bool Logic::function_stop (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_stop takes 1 argument");
  std::string target = args[0];
  if (target == "music")
  {
    remove("Game:music");
    emit ("Music:stop");
  }
  else
    emit (target + ":stop_animation");
  return true;
}

bool Logic::function_talk (const std::vector<std::string>& args)
{
  check (args.size() == 1 || args.size() == 2, "function_stop takes 1 or 2 arguments");
  std::string id;
  std::string text;

  if (args.size() == 1)
  {
    id = value<C::String>("Player:name");
    text = args[0];
  }
  else
  {
    check (args.size() == 2, "\"comment\" expects 1 or 2 arguments ("
           + std::to_string(args.size()) + "given)");
    id = args[0];
    text = args[1];
  }

  text = locale(text);
  if (id == "Hinter")
    text = "*" + text + "*";

  std::vector<C::Image_handle> dialog;
  create_dialog (id, text, dialog);

  int nb_char = int(text.size());
  double nb_seconds_read
      = (value<C::Int>("Dialog:speed") / double(Config::MEDIUM_SPEED))
      * (Config::min_reading_time + nb_char * Config::char_spoken_time);
  double nb_seconds_lips_moving = nb_char * Config::char_spoken_time;

  Point position = value<C::Double>(CAMERA__ZOOM)
                   * (value<C::Position>(id + "_body:position") - value<C::Position>(CAMERA__POSITION));

  int x = position.X();

  auto img = request<C::Image>(id + "_body:image");
  if (!img)
    img = get<C::Image>(value<C::String>("Player:name") + "_body:image");

  int y = position.Y() - value<C::Double>(CAMERA__ZOOM) * img->height() * img->scale() * 1.2;

  double size_factor = 0.75 * (value<C::Int>("Dialog:size") / double(Config::MEDIUM));

  for (auto img : dialog)
    if (x + size_factor * img->width() / 2 > int(0.95 * Config::world_width))
      x = int(0.95 * Config::world_width - size_factor * img->width() / 2);
    else if (x - size_factor * img->width() / 2 < int(0.1 * Config::world_width))
      x = int(0.1 * Config::world_width + size_factor * img->width() / 2);

  std::reverse(dialog.begin(), dialog.end());

  for (auto img : dialog)
  {
    auto pos = set<C::Absolute_position> (img->entity() + ":position", Point(x,y));
    y -= 80 * size_factor;

   m_current_action->schedule (m_current_time + std::max(1., nb_seconds_read), img);
   m_current_action->schedule (m_current_time + std::max(1., nb_seconds_read), pos);
  }

  if (id != "Hinter")
  {
    emit (id + ":start_talking");

   m_current_action->schedule (m_current_time + nb_seconds_lips_moving,
                                    C::make_handle<C::Signal>
                                    (id + ":stop_talking"));
  }
  return true;
}

bool Logic::function_trigger (const std::vector<std::string>& args)
{
  check (!args.empty(), "function_trigger takes at least 1 argument");
  std::string id = args[0];
  // Dialog
  if (request<C::Dialog>(id + ":dialog"))
    return subfunction_trigger_dialog (args);

  // Action
  if (auto action = request<C::Action>(id + ":action"))
  {
    action->launch();
    return true;
  }

  // Hints
  if (id == "hints")
  {
    create_hints();
    return true;
  }

  // else Menu
  set<C::String>("Game:triggered_menu", id);
  emit("Show:menu");

  return true;
}

bool Logic::function_unlock (const std::vector<std::string>&)
{
  status()->pop();
  return true;
}

bool Logic::function_wait (const std::vector<std::string>& args)
{
  check (args.size() <= 1, "function_wait takes 0 or 1 argument");
  if (args.size() == 1)
  {
    double time = to_double(args[0]);
    debug << "Schedule wait until " << m_current_time + time << std::endl;
    m_current_action-> schedule (m_current_time + time, C::make_handle<C::Base>("wait"));
  }
  return false;
}

} // namespace Sosage::System
