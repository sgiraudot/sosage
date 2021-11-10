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

#define INIT_DISPATCHER(x) \
  m_dispatcher.insert (std::make_pair(#x, std::bind(&Logic::x, this, std::placeholders::_1)))

namespace Sosage::System
{

namespace C = Component;

Logic::Logic (Content& content)
  : Base (content), m_current_time(0)
{
  INIT_DISPATCHER(function_add);
  INIT_DISPATCHER(function_camera);
  INIT_DISPATCHER(function_dialog);
  INIT_DISPATCHER(function_goto);
  INIT_DISPATCHER(function_look);
  INIT_DISPATCHER(function_play);
  INIT_DISPATCHER(function_set);
  INIT_DISPATCHER(function_stop);
  INIT_DISPATCHER(function_system);
  INIT_DISPATCHER(function_talk);
  set<C::Action>("Logic:action");
}

void Logic::run ()
{
  start_timer();
  m_current_time = value<C::Double> (CLOCK__TIME);

  if (status()->is (CUTSCENE) || status()->was (CUTSCENE))
  {
    run_cutscene();
    stop_timer("Logic");
    return;
  }
  if (status()->is (PAUSED, DIALOG_CHOICE, IN_MENU))
  {
    stop_timer("Logic");
    return;
  }

  std::set<std::string> done;
  for (auto c : components("action"))
    if (auto a = C::cast<C::Action>(c))
      if (c->entity() != "Character")
      {
        a->update_scheduled
            ([&](const C::Action::Timed_handle& th) -> bool
        {
          if (th.first == 0) // special case for Path
          {
            auto saved_path = C::cast<C::Path>(th.second);
            auto current_path = request<C::Path>(saved_path->id());
            if (saved_path == current_path)
              return true;
            return false;
          }
          if (th.first <= m_current_time)
          {
            if (C::cast<C::Signal>(th.second))
              set (th.second);
            else if (th.second->id() != "wait")
              remove (th.second->id());
            return false;
          }
          // else
          return true;
        });
      }

  if (receive ("Cursor:clicked"))
  {
    compute_path_from_target(get<C::Position>(CURSOR__POSITION));

    // Cancel current action
    if (auto action = request<C::Action>("Character:action"))
    {
      auto logic_action = get<C::Action>("Logic:action");

      for (const auto& th : action->scheduled())
      {
        if (th.first == 0) // special case for Path
          continue;
        if (th.second->id().find("Comment_") == 0) // keep dialogs when moving
          logic_action->schedule (th.first, th.second);
        else
        {
          if (C::cast<C::Signal>(th.second))
            set (th.second);
          else
            remove (th.second->id());
        }
      }

      action->stop();
      remove("Character:action");
    }
  }
  if (receive ("Stick:moved"))
  {
    auto direction = get<C::Simple<Vector>>(STICK__DIRECTION);
    const std::string& id = value<C::String>("Player:name");
    if (direction->value() == Vector(0,0))
    {
      remove(id + ":path", true);
      emit(id + ":stop_walking");
    }
    else
    {
      auto position = get<C::Position>(id + "_body:position");

      // Make direction "flatter" (45° becomes 30°, etc.)
      Vector dir = direction->value();
      dir = Vector (dir.x() * 1.22, dir.y() / 1.41);
      compute_path_from_direction(dir);
    }
  }

  if (receive ("code:button_clicked"))
  {
    auto code = get<C::Code>("Game:code");
    auto window = get<C::Image>("Game:window");
    auto cropped
      = get<C::Cropped>(window->entity() + "_button:image");

    cropped->crop (code->xmin(), code->xmax(), code->ymin(), code->ymax());
    set<C::Absolute_position>
      (window->entity() + "_button:position",
       value<C::Position>(window->entity() + ":position")
       + Vector(code->xmin(), code->ymin()));

    cropped->on() = true;

    get<C::Action>("Logic:action")->schedule (m_current_time + Config::button_click_duration,
                                              C::make_handle<C::Signal>("code:stop_flashing"));

    if (code->failure())
    {
      code->reset();
      emit ("code:play_failure");
    }
    else if (code->success())
    {
      code->reset();
      emit ("code:play_success");
      get<C::Action>("Logic:action")->schedule (m_current_time + Config::button_click_duration,
                                                C::make_handle<C::Signal>("code:quit"));
    }
    else
      emit ("code:play_click");
  }

  if (receive ("code:stop_flashing"))
  {
    auto window = get<C::Image>("Game:window");
    auto cropped
      = get<C::Cropped>(window->entity() + "_button:image");
    cropped->on() = false;
  }

  if (receive ("code:quit"))
  {
    auto code = get<C::Code>("Game:code");
    emit("Interface:hide_window");
    code->reset();
    remove("Code_hover:image", true);
    status()->pop();

    get<C::Action>(get<C::Code>("Game:code")->entity() + ":action")->launch();
  }

  if (auto follower = request<C::String>("Follower:name"))
    follow (follower->value());

  if (auto new_room_origin = request<C::String>("Game:new_room_origin"))
  {
    get<C::Action>(new_room_origin->value() + ":action")->launch();
    remove ("Game:new_room_origin");
  }

  if (auto triggered_action = request<C::Action>("Character:triggered_action"))
  {
    if (auto action = request<C::Action>("Character:action"))
    {
      debug << "Action " << action->entity() << " interrupted" << std::endl;
      for (const auto& th : action->scheduled())
      {
        if (th.first != 0) // special case for Path
        {
          if (C::cast<C::Signal>(th.second))
            set (th.second);
          else if (th.second->id() != "wait")
            remove (th.second->id());
        }
      }
      action->stop();
    }
    triggered_action->launch();
    debug << "Action " << triggered_action->entity() << " launched" << std::endl;
    set<C::Variable>("Character:action", triggered_action);
    remove ("Character:triggered_action");
  }

  for (auto c : components("action"))
    if (auto a = C::cast<C::Action>(c))
      if (c->entity() != "Character")
      {
        if (!a->on() || !a->ready())
          continue;
        debug << "Applying steps of action " << a->id() << std::endl;
        do
        {
          if (!apply_next_step (a))
            break;
        }
        while (a->on());
      }

  update_debug_info (get<C::Debug>(GAME__DEBUG));
  stop_timer("Logic");
}

void Logic::run_cutscene()
{
  auto cutscene = get<C::Cutscene>("Game:cutscene");
  bool paused = status()->is (PAUSED, IN_MENU);
  double current_time
      = cutscene->current_time (m_current_time, paused);
  if (current_time < 0)
    return;

  bool skip = receive ("Game:skip_cutscene");
  if (skip)
    current_time = std::numeric_limits<double>::max();

  for (C::Cutscene::Element& el : *cutscene)
  {
    if (!el.active)
      continue;

    double begin_time = el.keyframes.front().time;
    if (begin_time > current_time)
      continue;

    if (el.keyframes.size() == 1) // load case
    {
      C::Base dummy (el.id);
      set<C::String>("Game:new_room", dummy.entity());
      set<C::String>("Game:new_room_origin", dummy.component());
      status()->pop();
      continue;
    }
    // If cutscene skipped, continue until a load case is reached
    if (skip)
      continue;

    double end_time = el.keyframes.back().time;
    if (end_time < current_time)
    {
      el.active = false;
      if (auto img = request<C::Image>(el.id))
        img->on() = false;
      else if (auto music = request<C::Music>(el.id))
      {
        remove ("Game:music");
        emit ("Music:stop");
      }
      else if (el.id == "fadein")
        get<C::Image>("Blackscreen:image")->on() = false;
      continue;
    }

    int x, y, z;
    double zoom;
    cutscene->get_frame (current_time, el, x, y, z, zoom);

    if (el.id == "fadein" || el.id == "fadeout")
    {
      auto img = get<C::Image>("Blackscreen:image");
      img->on() = true;
      if (el.id == "fadein")
        img->set_alpha((unsigned char)(255 * (1. - zoom)));
      else
        img->set_alpha((unsigned char)(255 * zoom));
    }
    else if (auto img = request<C::Image>(el.id))
    {
      if (auto anim = C::cast<C::Animation>(img))
      {
        if (!img->on())
          emit (img->entity() + ":start_animation");
        if (!anim->loop())
          el.active = false;
      }

      img->on() = true;
      img->z() = z;
      if (el.id.find("text") != 0)
        img->set_scale(zoom);
      set<C::Absolute_position>(img->entity() + ":position", Point(x,y));

    }
    else if (auto music = request<C::Music>(el.id))
    {
      if (!request<C::Variable>("Game:music"))
      {
        set<C::Variable>("Game:music", get<C::Music>(el.id));
        emit ("Music:start");
      }
    }
    else
    {
      check (false, "Can't find cutscene element " + el.id);
    }
  }
}

bool Logic::compute_path_from_target (C::Position_handle target,
                                      std::string id)
{
  auto ground_map = get<C::Ground_map>("Background:ground_map");

  if (id == "")
    id = value<C::String>("Player:name");
  auto position = get<C::Position>(id + "_body:position");

  Point origin = position->value();
  Point t = target->value();

  //debug("Target = ", t);

  if (target->component() != "view")
    t = t + value<C::Absolute_position>(CAMERA__POSITION);

  //debug("Computing path from ", origin, " to ", t);
  std::vector<Point> path;
  ground_map->find_path (origin, t, path);

  // Check if character is already at target
  if (path.empty() || ((path.size() == 1) && (path[0] == origin)))
    return false;

  auto p = set<C::Path>(id + ":path", path);
  if ((*p)[0] == origin)
    p->current() ++;
  return true;
}

bool Logic::compute_path_from_direction (const Vector& direction)
{
  auto ground_map = get<C::Ground_map>("Background:ground_map");
  const std::string& id = value<C::String>("Player:name");
  auto position = get<C::Position>(id + "_body:position");
  Point origin = position->value();

  std::vector<Point> path;

  ground_map->find_path (origin, direction, path);

  // Check if character is already at target
  if (path.empty() || ((path.size() == 1) && (path[0] == origin)))
    return false;

  auto p = set<C::Path>(id + ":path", path);
  if ((*p)[0] == origin)
    p->current() ++;
  return true;
}

void Logic::follow (const std::string& follower)
{
  const std::string& player = value<C::String>("Player:name");

  auto pos_player = get<C::Position>(player + "_idle:position");
  auto pos_follower = get<C::Position>(follower + "_idle:position");

  double dx = std::abs(pos_player->value().x() - pos_follower->value().x());
  double dy = std::abs(pos_player->value().y() - pos_follower->value().y());

  bool is_moving = get<C::Image>(follower + "_walking:image")->on();

  double z = get<C::Image>(player + "_walking:image")->z();

  int reach_x = Config::object_reach_x * z / Config::follow_factor;
  int reach_y = Config::object_reach_y * z / Config::follow_factor;
  int reach_hysteresis = 0;
  if (!is_moving)
    reach_hysteresis = Config::object_reach_hysteresis * z / Config::follow_factor;

  std::cerr << reach_x << " " << reach_y << " " << reach_hysteresis << std::endl;

  // Object out of reach
  if (dx > reach_x + reach_hysteresis ||
      dy > reach_y + reach_hysteresis)
    compute_path_from_target (pos_player, follower);
  else
  {
    remove(follower + ":path", true);
    emit(follower + ":stop_walking");
  }
}

void Logic::update_debug_info (C::Debug_handle debug_info)
{
  if (debug_info->value())
  {
    auto debug_font = get<C::Font> ("Debug:font");
    auto dbg_img = set<C::Image> ("Debug:image",
                                                    debug_font, "FF0000",
                                                    debug_info->debug_str());
    auto dbg_pos = set<C::Absolute_position>("Debug:position", Point(0,0));
  }
  else
  {
    auto dbg_img = request<C::Image> ("Debug:image");
    if (dbg_img)
      dbg_img->on() = false;

  }

}

bool Logic::apply_next_step (C::Action_handle action)
{
  m_current_action = action;
  const C::Action::Step& s = action->next_step();
  debug << m_current_time << ", applying " << s.to_string() << std::endl;
  check (m_dispatcher.find(s.function()) != m_dispatcher.end(),
         s.function() + " is not a valid function");
  return m_dispatcher[s.function()](s.args());
}

bool Logic::function_add (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_add() takes 2 arguments");
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
  std::string option = args[0];
  if (option == "fadein" || option == "fadeout")
  {
    check (args.size() == 2, "function_camera(fadein/fadeout) takes 1 argument");
    bool fadein = (option == "fadein");
    double duration = to_double(args[1]);
    auto begin = set<C::Double> ("Fade:begin", m_current_time);
    auto end = set<C::Double> ("Fade:end", m_current_time + duration);
    auto out = set<C::Boolean> ("Fade:in", fadein);
    if (request<C::Music>("Game:music"))
      emit ("Music:fade");
    m_current_action->schedule (m_current_time + duration, begin);
    m_current_action->schedule (m_current_time + duration, end);
    m_current_action->schedule (m_current_time + duration, out);
  }
  else if (option == "shake")
  {
    check (args.size() == 3, "function_camera(shake) takes 2 arguments");
    double intensity  = to_double(args[1]);
    double duration = to_double(args[2]);
    auto begin = set<C::Double> ("Shake:begin", m_current_time);
    auto end = set<C::Double> ("Shake:end", m_current_time + duration);
    auto intens = set<C::Double> ("Shake:intensity", intensity);
    auto camera = set<C::Double> ("Camera:saved_position", value<C::Absolute_position>(CAMERA__POSITION).x());
    m_current_action->schedule (m_current_time + duration, begin);
    m_current_action->schedule (m_current_time + duration, end);
    m_current_action->schedule (m_current_time + duration, intens);
    m_current_action->schedule (m_current_time + duration, camera);

  }
  else if (option == "target")
  {
    check (args.size() == 2, "function_camera(target) takes 1 arguments");
    int target = to_int(args[1]);
    auto position = get<C::Position>(CAMERA__POSITION);

    set<C::GUI_position_animation>("Camera:animation", m_current_time, m_current_time + Config::camera_speed,
                                   position, Point (target, position->value().y()));
  }
  return true;
}

bool Logic::function_dialog (const std::vector<std::string>& args)
{
  check (!args.empty(), "function_dialog() takes 1 argument");
  auto dialog = get<C::Dialog>(args[0] + ":dialog");

  auto action = C::make_handle<C::Action>("Dialog:action");

  if (args.size() == 1)
  {
    status()->push(LOCKED);
    if (auto pos = request<C::Int>("Saved_game:dialog_position"))
    {
      dialog->init (pos->value());
      remove ("Saved_game:dialog_position");
    }
    else
      dialog->init();
  }
  else if (auto choice = request<C::Int>("Dialog:choice"))
  {
    action->add ("talk",
    { get<C::Vector<std::string> >("Dialog:choices")
      ->value()[std::size_t(choice->value())] });
    action->add ("system", { "wait" });
    dialog->next(choice->value());
    remove("Dialog:choice");
  }
  else
  {
    dialog->next();
  }

  if (dialog->is_over())
  {
    status()->pop();
    if (dialog->line().first != "")
    {
      set<C::Variable>("Character:triggered_action", get<C::Action>(dialog->line().first + ":action"));
      return false;
    }
  }
  else if (dialog->is_line())
  {
    std::string character;
    std::string line;
    std::tie (character, line) = dialog->line();
    action->add ("talk", { character, line });
    action->add ("system", { "wait" });
    action->add ("dialog", { args[0], "continue" });
  }
  else
  {
    // Keep track in case player saves and reload there
    set<C::Int>("Game:dialog_position", dialog->current());

    status()->push(DIALOG_CHOICE);
    auto choices = set<C::Vector<std::string> >("Dialog:choices");
    dialog->get_choices (*choices);
    action->add ("dialog", { args[0], "continue" });

    // Keep track in case player saves and reload there
    set<C::String>("Game:current_dialog", args[0]);
  }

  if (action->size() != 0)
    set<C::Variable>("Character:triggered_action", action);

  return false;
}

bool Logic::function_goto (const std::vector<std::string>& init_args)
{
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

bool Logic::function_look (const std::vector<std::string>& args)
{
  check (args.size() <= 1, "function_goto() takes at most 1 argument");

  std::string target = "";
  if (args.size() == 1)
    target = args[0];
  else
    target = get<C::Action>("Character:action")->target_entity();

  debug << "Action_look " << target << std::endl;
  const std::string& id = value<C::String>("Player:name");

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

bool Logic::function_play (const std::vector<std::string>& args)
{
  std::string option = args[0];
  std::string target = args[1];

  if (option == "animation")
  {
    check (args.size() == 3 || args.size() == 2, "function_play(animation) takes 1 or 2 arguments");
    if (args.size() == 3) // Target is character
    {
      const std::string& character = value<C::String>("Player:name");

      double duration = to_double(args[2]);
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
  }
  else if (option == "sound")
    emit (target + ":play_sound");
  else if (option == "music")
  {
    set<C::Variable>("Game:music", get<C::Music>(target + ":music"));
    emit ("Music:start");
  }
  return true;
}

bool Logic::function_set (const std::vector<std::string>& args)
{
  std::string option = args[0];
  std::string target = args[1];

  if (option == "coordinates")
  {
    int x = to_int(args[2]);
    int y = to_int(args[3]);
    int z = to_int(args[4]);
    if (args.size() == 6) // Smooth move
    {
      double duration = to_double(args[5]);
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
  }
  else if (option == "follower")
  {
    if (target == "")
      remove ("Follower:name");
    else
      set<C::String>("Follower:name", target);
  }
  else if (option == "player")
    set<C::String>("Player:name", target);
  else if (option == "state")
  {
    auto current_state = get<C::String>(target + ":state");
    std::string state;

    if (args.size() == 3)
      state = args[2];
    else // if (step.size() == 4)
    {
      if (args[2] != current_state->value())
        return true;
      state = args[3];
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
  }
  else if (option == "visible")
  {
    if (auto boolean = request<C::Boolean>(target + ":visible"))
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
  }
  else if (option == "hidden")
  {
    if (auto question = request<C::String>(target + ":question"))
      get<C::Set<std::string>>("Hints:list")->erase(target);
    else
      emit (target + ":set_hidden");
  }

  return true;
}

bool Logic::function_stop (const std::vector<std::string>& args)
{
  std::string option = args[0];
  if (option == "animation")
  {
    std::string target = args[1];
    emit (target + ":stop_animation");
  }
  else if (option == "music")
  {
    remove("Game:music");
    emit ("Music:stop");
  }
  return true;
}

bool Logic::function_system (const std::vector<std::string>& args)
{
  std::string option = args[0];
  if (option == "load")
  {
    set<C::String>("Game:new_room",args[1]);
    set<C::String>("Game:new_room_origin", args[2]);
  }
  else if (option == "lock")
    status()->push(LOCKED);
  else if (option == "trigger")
  {
    std::string id = args[1];
    set<C::Variable>("Character:triggered_action", get<C::Action>(id + ":action"));
  }
  else if (option == "menu")
  {
    set<C::String>("Game:triggered_menu", args[1]);
    emit("Show:menu");
  }
  else if (option == "unlock")
    status()->pop();
  else if (option == "wait")
  {
    if (args.size() == 2)
    {
      double time = to_double(args[1]);
      debug << "Schedule wait until " << m_current_time + time << std::endl;
      m_current_action-> schedule (m_current_time + time, C::make_handle<C::Base>("wait"));
    }
    return false;
  }
  else if (option == "hints")
    create_hints();
  else if (option == "exit")
    emit ("Game:exit");

  return true;
}

bool Logic::function_talk (const std::vector<std::string>& args)
{
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

  std::vector<C::Image_handle> dialog;
  create_dialog (id, text, dialog);

  int nb_char = int(text.size());
  double nb_seconds_read
      = (value<C::Int>("Dialog:speed") / double(Config::MEDIUM_SPEED))
      * (Config::min_reading_time + nb_char * Config::char_spoken_time);
  double nb_seconds_lips_moving = nb_char * Config::char_spoken_time;

  int y = 100;
  int x = int(value<C::Position>(id + "_body:position").x()
              - value<C::Position>(CAMERA__POSITION).x());

  double size_factor = 0.75 * (value<C::Int>("Dialog:size") / double(Config::MEDIUM));

  for (auto img : dialog)
    if (x + size_factor * img->width() / 2 > int(0.95 * Config::world_width))
      x = int(0.95 * Config::world_width - size_factor * img->width() / 2);
    else if (x - size_factor * img->width() / 2 < int(0.1 * Config::world_width))
      x = int(0.1 * Config::world_width + size_factor * img->width() / 2);

  for (auto img : dialog)
  {
    auto pos = set<C::Absolute_position> (img->entity() + ":position", Point(x,y));
    y += 80 * size_factor;

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

void Logic::create_dialog (const std::string& character,
                           const std::string& text,
                           std::vector<C::Image_handle>& dialog)
{
  static const int width_max = int(0.6 * Config::world_width);

  double size_factor = 0.75 * (value<C::Int>("Dialog:size") / double(Config::MEDIUM));

  auto font = get<C::Font> ("Dialog:font");
  const std::string& color = value<C::String> (character + ":color");

  auto img
    = set<C::Image> ("Comment:image",
                     font,
                     color,
                     text, true);
  img->set_scale(size_factor);
  img->set_relative_origin(0.5, 0.5);

  if (img->width() <= width_max)
    dialog.push_back (img);
  else
  {
    remove("Comment:image");
    int nb_imgs = 1 + (img->width() / width_max);

    // Find space characters where to cut
    std::vector<std::size_t> cuts (std::size_t(nb_imgs  - 1));

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
      std::size_t begin = (i == 0 ? 0 : cuts[std::size_t(i-1)] + 1);
      std::size_t end = (i == nb_imgs - 1 ? text.size() : cuts[std::size_t(i)]);
      auto img
        = set<C::Image> ("Comment_" + std::to_string(i) + ":image",
                         font,
                         color,
                         std::string(text.begin() + std::ptrdiff_t(begin),
                                     text.begin() + std::ptrdiff_t(end)), true);
      img->z() = Config::inventory_depth;
      img->set_scale(size_factor);
      img->set_relative_origin(0.5, 0.5);
      dialog.push_back (img);
    }
  }

}

void Logic::create_hints()
{
  auto dialog = set<C::Dialog>("Hints:dialog", "End_hints");
  set<C::String>("Hinter:color", "FFFFFF");
  const std::string& player = value<C::String>("Player:name");
  set<C::Variable>("Hinter_body:position", get<C::Position>(player + "_body:position"));

  auto first = dialog->add_vertex ("Hinter", "*" + locale_get("Hint_welcome:text") + "*");
  auto choice = dialog->add_vertex();
  dialog->add_edge(dialog->vertex_in(), first);
  dialog->add_edge(first, choice);

  for (const std::string& h : get<C::Set<std::string>>("Hints:list")->value())
  {
    auto va = dialog->add_vertex ("Hinter", "*" + locale_get (h + ":answer") + "*");
    dialog->add_edge(choice, va, true, locale_get (h + ":question"));
    dialog->add_edge(va, choice);
  }

  auto closing = dialog->add_vertex ("Hinter", "*" + locale_get("Hint_bye:text") + "*");
  dialog->add_edge(choice, closing, false, locale_get("Hint_end:text"));
  dialog->add_edge(closing, dialog->vertex_out());

  emit(player + ":stop_walking");
  remove(player + ":path", true);
  auto action = set<C::Action>("Hints:action");
  set<C::Variable>("Character:action", action);
  action->add("play", { "animation", "telephone", "-1" });
  action->add("dialog", { "Hints" });

  auto end = set<C::Action>("End_hints:action");
  end->add("stop", { "animation", player });

  set<C::String>(player + ":start_animation", "telephone");
}


} // namespace Sosage::System
