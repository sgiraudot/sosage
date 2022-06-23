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
#include <Sosage/Component/Dialog.h>
#include <Sosage/Component/Group.h>
#include <Sosage/Component/GUI_animation.h>
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

/*
  - add: [ID integer_id, INT diff]  -> adds diff to the value of integer_id
  - add: [ID list_id, ID action_id] -> appends action_id to list_id
 */
bool Logic::function_add (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_add takes 2 arguments");
  std::string id = args[0];

  auto integer = request<C::Int>(id , "value");
  if (integer)
  {
    int diff = to_int(args[1]);
    integer->set (integer->value() + diff);

    auto action = request<C::Action>(id, std::to_string(integer->value()));
    if (!action)
      action = request<C::Action>(id , "default");

    if (action)
    {
      action->launch();
      while (action->on())
        apply_next_step (action);
    }
  }
  else
  {
    auto list = request<C::Vector<std::string>>(id , "list");
    if (!list)
      list = set<C::Vector<std::string>>(id , "list");
    list->push_back (args[1]);
  }
  return true;
}

/*
  - camera: [INT x]                        -> Quick smooth scrolling to (x,0)
  - camera: [INT x, INT y]                 -> Quick smooth scrolling to (x,y)
  - camera: [INT x, INT y, FLOAT duration] -> Smooth scrolling to (x,y) with wanted duration
 */
bool Logic::function_camera (const std::vector<std::string>& args)
{
  check (1 <= args.size() && args.size() <= 3, "function_camera takes 1, 2 or 3 arguments");
  int xtarget = to_int(args[0]);
  int ytarget = (args.size() > 1 ? to_int(args[1]) : 0);
  auto position = get<C::Position>(CAMERA__POSITION);

  if (args.size() == 3)
  {
    double duration = to_double(args[2]);
    if (duration == 0.)
      position->set (Point (xtarget, ytarget));
    else
    {
      double begin_time = m_current_time;
      double end_time = begin_time + duration;

      m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy", "event"));

      auto anim = set<C::Tuple<Point, Point, double, double>>
          ("Camera", "move60fps", position->value(), Point(xtarget, ytarget),
           begin_time, end_time);
      m_current_action->schedule (end_time, anim);
    }
  }
  else
    set<C::GUI_position_animation>("Camera", "animation", m_current_time, m_current_time + Config::camera_speed,
                                   position, Point (xtarget, ytarget));
  return true;
}

/*
  - control: [ID player_id]                 -> controls player_id and remove follower if any
  - control: [ID player_id, ID follower_id] -> controls player_id and make follower_id follow
 */
bool Logic::function_control (const std::vector<std::string>& args)
{
  check (args.size() == 1 or args.size() == 2, "function_load control takes 1 or 2 arguments");
  std::string leader = args[0];
  set<C::String>("Player", "name", leader);

  if (args.size() == 2)
  {
    std::string follower = args[1];
    set<C::String>("Follower", "name", follower);
  }
  else
    remove ("Follower", "name", true);
  return true;
}

/*
  - cutscene: [] -> locks interface for cutscene
 */
bool Logic::function_cutscene (const std::vector<std::string>&)
{
  status()->push(CUTSCENE);
  return true;
}

/*
  - exit: [] -> exits the game
 */
bool Logic::function_exit (const std::vector<std::string>&)
{
  emit ("Game", "exit");
  return true;
}

/*
  - fadein: [FLOAT duration] -> camera fades in with the wanted duration
 */
bool Logic::function_fadein (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_fadein takes 1 argument");
  return subfunction_fade (true, to_double(args[0]));
}

/*
  - fadeout: [FLOAT duration] -> camera fades out with the wanted duration
 */
bool Logic::function_fadeout (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_fadein takes 1 argument");
  return subfunction_fade (false, to_double(args[0]));
}

/*
  - goto: []                              -> player goes to current object
  - goto: [ID target_id]                  -> player goes to target_id
  - goto: [INT x, INT y]                  -> player goes to coordinates (x,y)
  - goto: [ID character_id, ID target_id] -> character goes to target_id
  - goto: [ID character_id, INT x, INT y] -> character goes to coordinates (x,y)
 */
bool Logic::function_goto (const std::vector<std::string>& init_args)
{
  check (init_args.size() <= 3, "function_goto takes at most 3 arguments");
  std::string id = "";
  std::vector<std::string> args;
  if (init_args.size() > 0 && request<C::Image>(init_args[0] + "_head", "image"))
  {
    id = init_args[0];
    args = std::vector<std::string>(init_args.begin() + 1, init_args.end());
  }
  else
  {
    id = value<C::String>("Player", "name");
    args = init_args;
  }

  if (args.size() == 2)
  {
    if (compute_path_from_target
        (C::make_handle<C::Absolute_position>("Goto", "view", Point (to_int(args[0]), to_int(args[1]))),
         id))
     m_current_action->schedule (0, get<C::Path>(id , "path"));
  }
  else
  {
    std::string target
        = (args.empty() ? get<C::Action>("Character", "action")->target_entity() : args[0]);
    debug << "Action_goto " << target << std::endl;

    auto position = request<C::Position>(target , "view");
    if (compute_path_from_target(position, id))
     m_current_action->schedule (0, get<C::Path>(id , "path"));
  }

  return true;
}

/*
  - hide: [ID hint_id]   -> makes hint unavailable
  - hide: [ID target_id] -> makes target_id invisible
 */
bool Logic::function_hide (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_hide takes 1 argument");
  std::string target = args[0];
  if (auto question = request<C::String>("Hint_" + target , "question"))
    get<C::Set<std::string>>("Hints", "list")->erase("Hint_" + target);
  else if (request<C::Group>(target , "group"))
    emit (target , "set_hidden");
  else
    get<C::Image>(target , "image")->on() = false;
  return true;
}

/*
  - load: [ID room_id, ID origin_id] -> loads room and triggers wanted origin
 */
bool Logic::function_load (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_load takes 2 arguments");
  set<C::String>("Game", "new_room", args[0]);
  set<C::String>("Game", "new_room_origin", args[1]);
  if (status()->is(CUTSCENE))
    status()->pop();
  status()->push(LOCKED);
  return true;
}

/*
  - lock: [] -> locks interface
 */
bool Logic::function_lock (const std::vector<std::string>&)
{
  status()->push(LOCKED);
  return true;
}

/*
  - look: []                              -> player looks at current object
  - look: [ID target_id]                  -> player looks at target_id
  - look: [ID character_id, ID target_id] -> character looks at target_id
 */
bool Logic::function_look (const std::vector<std::string>& args)
{
  check (args.size() <= 2, "function_look takes at most 2 argument");

  std::string target = "";
  std::string id = "";
  if (args.empty())
  {
    id = value<C::String>("Player", "name");
    target = get<C::Action>("Character", "action")->target_entity();
  }
  else if (args.size() == 1)
  {
    id = value<C::String>("Player", "name");
    target = args[0];
  }
  else // if (args.size() == 2)
  {
    id = args[0];
    target = args[1];
  }

  debug << "Action_look " << id << " " << target << std::endl;

  if (target == "default" || !request<C::Position>(target , "position"))
    set<C::Absolute_position>(id , "lookat",
                              value<C::Position>(CURSOR__POSITION));
  else
  {
    auto state = request<C::String>(target , "state");
    if (!state || state->value() != "inventory")
      set<C::Absolute_position>(id , "lookat",
                                value<C::Position>(target , "position"));
  }
  return true;
}

/*
  - loop: [] -> goes back to the beginning of current action
 */
bool Logic::function_loop (const std::vector<std::string>&)
{
  m_current_action->stop();
  m_current_action->launch();
  return true;
}

/*
  - message: [ID text_id] -> display a message to the player
 */
bool Logic::function_message (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_message takes 1 argument");
  set<C::Variable>("Message", "text", get<C::String>(args[0], "text"));
  set<C::String>("Game", "triggered_menu", "Message");
  emit("Show", "menu");
  return false;
}

/*
  - move: [ID character_id, INT x, INT y, BOOL looking_right]        -> immediately moves character to the coordinates (x,y), looking right/left
  - move: [ID character_id, INT x, INT y, INT z, BOOL looking_right] -> immediately moves character to the coordinates (x,y,z), looking right/left
  - move: [ID target_id, INT x, INT y, INT z]                        -> immediately moves target to the coordinates (x,y,z)
  - move: [ID target_id, INT x, INT y, INT z, FLOAT duration]        -> smoothly moves target to coordinates (x,y,z) with wanted duration
 */
bool Logic::function_move (const std::vector<std::string>& args)
{
  check (args.size() == 4 || args.size() == 5, "function_move takes 4 or 5 arguments");
  std::string target = args[0];
  int x = to_int(args[1]);
  int y = to_int(args[2]);

  if (request<C::Group>(target , "group")) // character
  {
    bool looking_right;
    if (args.size() == 4)
      looking_right = to_bool(args[3]);
    else
    {
      int z = to_int(args[3]);
      looking_right = to_bool(args[4]);
      set<C::Int>(target , "z", z);
    }

    get<C::Absolute_position>(target + "_body", "position")->set(Point(x, y));
    set<C::Boolean>(target , "looking_right", looking_right);
  }
  else
  {
    int z = to_int(args[3]);
    if (args.size() == 5) // Smooth move
    {
      double duration = to_double(args[4]);
      int nb_frames = round (duration * Config::animation_fps);

      double begin_time = frame_time(m_current_time);
      double end_time = begin_time + (nb_frames + 0.5) / double(Config::animation_fps);

      m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy", "event"));

      auto pos = get<C::Position>(target , "position");

      auto anim = set<C::Tuple<Point, Point, double, double>>
          (target , "move", pos->value(), Point(x,y), begin_time, end_time);
      m_current_action->schedule (end_time,  anim);
    }
    else
    {
      get<C::Position>(target , "position")->set (Point(x, y));
      get<C::Image>(target , "image")->z() = z;
    }
  }
  return true;
}

/*
  - move60fps: [ID target_id, INT x, INT y, INT z, FLOAT duration] -> smoothly moves (GUI fps) target to coordinates (x,y,z) with wanted duration
 */
bool Logic::function_move60fps (const std::vector<std::string>& args)
{
  check (args.size() == 4 || args.size() == 5, "function_move takes 4 or 5 arguments");
  std::string target = args[0];
  int x = to_int(args[1]);
  int y = to_int(args[2]);
  //int z = to_int(args[3]);
  double duration = to_double(args[4]);
  double begin_time = m_current_time;
  double end_time = begin_time + duration;

  m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy", "event"));

  auto pos = get<C::Position>(target , "position");

  auto anim = set<C::Tuple<Point, Point, double, double>>
      (target , "move60fps", pos->value(), Point(x,y), begin_time, end_time);
  m_current_action->schedule (end_time,  anim);
  return true;
}

/*
  - play: [ID animation_id]                           -> starts animation
  - play: [ID music_id]                               -> starts music
  - play: [ID sound_id]                               -> plays sound
  - play: [ID music_id, FLOAT duration]               -> fadein music
  - play: [ID character_animation_id, FLOAT duration] -> plays animation of player character for the wanted duration
 */
bool Logic::function_play (const std::vector<std::string>& args)
{
  check (args.size() == 2 || args.size() == 1, "function_play takes 1 or 2 arguments");
  std::string target = args[0];

  if (request<C::Base>(target , "sound"))
  {
    emit (target , "play_sound");
    auto panning = set<C::Double>(target, "panning", 0.5);
    if (auto entity = request<C::Action>("Character", "action"))
    {
      if (auto pos = request<C::Position>(entity->target_entity(), "position"))
      {
        Point point = pos->value() - value<C::Absolute_position>(CAMERA__POSITION);
        panning->set(1. - (point.x() / Config::world_width));
      }
    }

    return true;
  }
  if (auto music = request<C::Music>(target , "music"))
  {
    set<C::Variable>("Game", "music", music);
    if (args.size() == 2)
    {
      double duration = to_double(args[1]);
      set<C::Tuple<double, double, bool>>
          ("Music", "fade",
           m_current_time, m_current_time + duration, true);
    }
    else
      emit ("Music", "start");
    return true;
  }

  // else, animation
  if (args.size() == 2) // Target is character
  {
    const std::string& character = value<C::String>("Player", "name");

    double duration = to_double(args[1]);
    set<C::String>(character , "start_animation", target);

    if (duration > 0)
      m_current_action->schedule (m_current_time + duration,
                                  C::make_handle<C::Signal>
                                  (character , "stop_animation"));
  }
  else
  {
    auto animation = get<C::Animation>(target , "image");
    emit (target , "start_animation");

    if (animation->loop())
    {
      // If animation loop, add a fake state so that it's reloaded
      // on if we exit and reenter the room
      set<C::String>(target , "state", "Dummy");
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

      m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy", "event"));
    }
  }
  return true;
}

/*
  - rescale: [ID target_id, FLOAT scale]                 -> rescales immediately target to the wanted scale
  - rescale: [ID target_id, FLOAT scale, FLOAT duration] -> rescales smoothly target to the wanted scale with the wanted duration
 */
bool Logic::function_rescale (const std::vector<std::string>& args)
{
  check (args.size() == 2 || args.size() == 3, "function_rescale takes 2 or 3 arguments");
  std::string target = args[0];
  double scale = to_double(args[1]);

  if (args.size() == 3) // Smooth rescale
  {
    double duration = to_double(args[2]);
    int nb_frames = round (duration * Config::animation_fps);

    double begin_time = frame_time(m_current_time);
    double end_time = begin_time + (nb_frames + 0.5) / double(Config::animation_fps);

    m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy", "event"));

    auto img = get<C::Image>(target , "image");

    auto anim = set<C::Tuple<double, double, double, double>>
        (target , "rescale", img->scale(), scale, begin_time, end_time);
   m_current_action->schedule (end_time,  anim);
  }
  else
    get<C::Image>(target , "image")->set_scale(scale);
  return true;
}

/*
  - rescale60fps: [ID target_id, FLOAT scale, FLOAT duration] -> rescales smoothly target to the wanted scale with the wanted duration
 */
bool Logic::function_rescale60fps (const std::vector<std::string>& args)
{
  check (args.size() == 3, "function_rescale takes 2 or 3 arguments");
  std::string target = args[0];
  double scale = to_double(args[1]);
  double duration = to_double(args[2]);
  double begin_time = m_current_time;
  double end_time = begin_time + duration;

  m_current_action->schedule (end_time, C::make_handle<C::Signal>("Dummy", "event"));

  auto img = get<C::Image>(target , "image");

  auto anim = set<C::Tuple<double, double, double, double>>
      (target , "rescale60fps", img->scale(), scale, begin_time, end_time);
  m_current_action->schedule (end_time,  anim);

  return true;
}

/*
  - set: [ID target_id, ID state_id]                      -> change state of tarfget to state_id
  - set: [ID target_id, ID state_from_id, ID state_to_id] -> change state of target to state_to_id ONLY if current state is state_from_id
 */
bool Logic::function_set (const std::vector<std::string>& args)
{
  check (args.size() == 2 || args.size() == 3, "function_set takes 2 or 3 arguments");
  std::string target = args[0];
  auto current_state = get<C::String>(target , "state");
  std::string state = args[1];

  if (args.size() == 3)
  {
    if (state != current_state->value())
      return true;
    state = args[2];
  }

  if (current_state->value() == "inventory")
  {
    get<C::Inventory>("Game", "inventory")->remove(target);
    //get<C::Absolute_position>(target , "position")->absolute() = false;
  }

  current_state->set (state);
  if (state == "inventory")
  {
    get<C::Inventory>("Game", "inventory")->add(target);
    auto img
        = get<C::Image>(target , "image");
    img->set_relative_origin(0.5, 0.5);
    img->z() = Config::inventory_depth;
    img->on() = false;
  }

  // Changing the state an object might change the labels of its
  // related action, force an update if using gamepad
  if (value<C::Simple<Input_mode>>(INTERFACE__INPUT_MODE) == GAMEPAD)
    emit("Action_selector", "force_update");

  return true;
}

/*
  - shake: [FLOAT intensity, FLOAT duration] -> shakes the camera with wanted intensity and duration
 */
bool Logic::function_shake (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_shake takes 2 arguments");
  double intensity  = to_double(args[0]);
  double duration = to_double(args[1]);
  auto shake = set<C::Array<double,4>>("Camera", "shake", m_current_time, m_current_time + duration,
                                       intensity, value<C::Absolute_position>(CAMERA__POSITION).x());
  m_current_action->schedule (m_current_time + duration, shake);
  return true;
}

/*
  - show: [ID hint_id] -> makes hint available
  - show: [ID target_id] -> makes target visible
 */
bool Logic::function_show (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_show takes 2 arguments");
  std::string target = args[0];
  if (auto group = request<C::Group>(target , "group")) // Character
    emit (target , "set_visible");
  else if (auto question = request<C::String>("Hint_" + target , "question"))
    get<C::Set<std::string>>("Hints", "list")->insert ("Hint_" + target);
  else
  {
    auto image = get<C::Image>(target , "image");
    if (image->z() == Config::interface_depth) // window
    {
      set<C::Variable>("Game", "window", image);
      emit ("Interface", "show_window");

      auto code = request<C::Code>(target , "code");
      if (code)
      {
        status()->push (IN_CODE);
        set<C::Variable>("Game", "code", code);
      }
      else
        status()->push (IN_WINDOW);
    }
    else
      image->on() = true;

  }
  return true;
}

/*
  - stop: ["music"]         -> stops the music
  - stop: [ID character_id] -> stops the character's current animation
 */
bool Logic::function_stop (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_stop takes 1 argument");
  std::string target = args[0];
  if (target == "music")
  {
    remove("Game", "music");
    emit ("Music", "stop");
  }
  else
    emit (target , "stop_animation");
  return true;
}

/*
  - talk: [STRING line]                                  -> makes player say the line
  - talk: [ID character_id, STRING line]                 -> makes character say the line
  - talk: [ID character_id, STRING line, FLOAT duration] -> makes character say the line for the wanted duration
 */
bool Logic::function_talk (const std::vector<std::string>& args)
{
  SOSAGE_TIMER_START(Logic__function_talk);
  check (1 <= args.size() && args.size() <= 3, "function_talk takes 1, 2 or 3 arguments");
  std::string id;
  std::string text;

  if (args.size() == 1)
  {
    id = value<C::String>("Player", "name");
    text = args[0];
  }
  else
  {
    id = args[0];
    text = args[1];
  }

  text = locale(text);
  if (id == "Hinter")
    text = "*" + text + "*";

  std::vector<C::Image_handle> dialog;
  create_dialog (id, text, dialog);

  int nb_char = int(text.size());
  double nb_seconds_read;
  if (args.size() == 3)
    nb_seconds_read = to_double(args[2]);
  else
    nb_seconds_read = (value<C::Int>("Dialog", "speed") / double(Config::MEDIUM_SPEED))
                      * (Config::min_reading_time + nb_char * Config::char_spoken_time);

  double nb_seconds_lips_moving = nb_char * Config::char_spoken_time;

  debug << "Line displayed for " << nb_seconds_read << " s, lips moving for "
        << nb_seconds_lips_moving << "s" << std::endl;

  Point position = value<C::Double>(CAMERA__ZOOM)
                   * (value<C::Position>(id + "_body", "position") - value<C::Position>(CAMERA__POSITION));

  int x = position.X();

  double y_diff = value<C::Int>(id , "dialog_height", 770);
  auto img = request<C::Image>(id + "_body", "image");
  if (!img)
    img = get<C::Image>(value<C::String>("Player", "name") + "_body", "image");

  y_diff *= img->scale() * value<C::Double>(CAMERA__ZOOM);

  int y = position.Y() - y_diff;
  double size_factor = 0.75 * (value<C::Int>("Dialog", "size") / double(Config::MEDIUM));

  int height = 80 * size_factor * dialog.size();
  y = y - (height / 2);
  if (y < 100)
    y = 100;

  for (auto img : dialog)
    if (x + size_factor * img->width() / 2 > int(0.95 * Config::world_width))
      x = int(0.95 * Config::world_width - size_factor * img->width() / 2);
    else if (x - size_factor * img->width() / 2 < int(0.1 * Config::world_width))
      x = int(0.1 * Config::world_width + size_factor * img->width() / 2);

  for (auto img : dialog)
  {
    auto pos = set<C::Absolute_position> (img->entity() , "position", Point(x,y));
    y += 80 * size_factor;

   m_current_action->schedule (m_current_time + std::max(1., nb_seconds_read), img);
   m_current_action->schedule (m_current_time + std::max(1., nb_seconds_read), pos);
  }

  if (id != "Hinter")
  {
    emit (id , "start_talking");

   m_current_action->schedule (m_current_time + nb_seconds_lips_moving,
                                    C::make_handle<C::Signal>
                                    (id , "stop_talking"));
  }
  SOSAGE_TIMER_STOP(Logic__function_talk);

  return true;
}

/*
  - timer: [ID timer_id] -> creates a timer
 */
bool Logic::function_timer (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_timer takes 1 argument");
  set<C::Double>(args[0] , "init_value", m_current_time);
  return true;
}

/*
  - trigger: [ID action_id] -> triggers the wanted action
  - trigger: [ID dialog_id] -> triggers the wanted dialog
  - trigger: [ID menu_id]   -> triggers the wanted menu
  - trigger: ["hints"]      -> triggers hints
 */
bool Logic::function_trigger (const std::vector<std::string>& args)
{
  check (!args.empty(), "function_trigger takes at least 1 argument");
  std::string id = args[0];
  // Dialog
  if (request<C::Dialog>(id , "dialog"))
    return subfunction_trigger_dialog (args);

  // Action
  if (auto action = request<C::Action>(id , "action"))
  {
    action->launch();

    // If trigger is from the character action, then the new action
    // becomes the character action
    auto char_action = request<C::Action>("Character", "action");
    if (m_current_action == char_action)
      set<C::Variable>("Character", "action", action);
    return true;
  }

  // Hints
  if (id == "hints")
  {
    create_hints();
    return true;
  }

  // else Menu
  set<C::String>("Game", "triggered_menu", id);
  emit("Show", "menu");

  return true;
}

/*
  - unlock: [] -> releases the interface to the user
 */
bool Logic::function_unlock (const std::vector<std::string>&)
{
  status()->pop();
  return true;
}

/*
  - wait: []                            -> waits until all ongoing events (talking, moving, etc.) are finished
  - wait: [FLOAT duration]              -> waits for the wanted duration
  - wait: [ID timer_id, FLOAT duration] -> waits for the wanted duration from the given timer
 */
bool Logic::function_wait (const std::vector<std::string>& args)
{
  check (args.size() <= 2, "function_wait takes 0 or 1 argument");
  if (args.size() == 1)
  {
    double time = to_double(args[0]);
//    debug << "Schedule wait until " << m_current_time + time << std::endl;
    m_current_action-> schedule (m_current_time + time, C::make_handle<C::Base>("wait", "wait"));
  }
  else if (args.size() == 2)
  {
    const std::string& id = args[0];
    double time = value<C::Double>(id , "init_value") + to_double(args[1]);
//    debug << "Schedule wait until " << time << std::endl;
    m_current_action->reset_scheduled();
    m_current_action->schedule (time, C::make_handle<C::Base>("wait", "wait"));
  }
  return false;
}

/*
  - zoom: [FLOAT value] -> change zoom to wanted value (1 = normal zoom)
 */
bool Logic::function_zoom (const std::vector<std::string>& args)
{
  check (args.size() == 1, "function_zoom takes 1 argument");
  double zoom = to_double(args[0]);
  get<C::Double>(CAMERA__ZOOM)->set(zoom);
  return true;
}

} // namespace Sosage::System
