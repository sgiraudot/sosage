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
  , m_current_action (nullptr), m_next_step (0)
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
}

void Logic::run ()
{
  m_current_time = get<C::Double> (CLOCK__TIME)->value();

  auto status = get<C::Status>(GAME__STATUS);
  if (status->value() == CUTSCENE || status->next_value() == CUTSCENE)
  {
    run_cutscene();
    return;
  }
  if (status->value() == PAUSED || status->value() == DIALOG_CHOICE ||
      status->value() == IN_MENU)
    return;
  std::set<Timed_handle> new_timed_handle;

  bool still_waiting // for this world to stop hating
      = true;

  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
    {
      auto saved_path = C::cast<C::Path>(th.second);
      auto current_path = request<C::Path>(saved_path->id());
      if (saved_path == current_path)
        new_timed_handle.insert(th);
    }
    else if (th.first <= m_current_time)
    {
      if (C::cast<C::Signal>(th.second))
        set (th.second);
      else if (th.second->id() == "wait")
        still_waiting = false;
      else
        remove (th.second->id());
    }
    else
      new_timed_handle.insert(th);
  m_timed.swap(new_timed_handle);

  auto collision = request<C::Image> ("Cursor:target");
  if (collision && receive ("Cursor:clicked"))
  {
    if (auto name = request<C::String>(collision->entity() + ":name"))
    {
      if (get<C::String> ("Chosen_verb:text")->entity() == "Verb_goto")
        compute_path_from_target(get<C::Position>(collision->entity() + ":view"));
    }
    else
      compute_path_from_target(get<C::Position>(CURSOR__POSITION));

    remove("Cursor:target");

    // Cancel current action
    clear_timed(true);
    m_current_action = nullptr;
  }

  if (receive ("code:button_clicked"))
  {
    auto code = get<C::Code>("Game:code");
    auto window = get<C::Image>("Game:window");
    auto cropped
      = get<C::Cropped>(window->entity() + "_button:image");

    cropped->crop (code->xmin(), code->xmax(), code->ymin(), code->ymax());
    set<C::Position>
      (window->entity() + "_button:position",
       get<C::Position>(window->entity() + ":position")->value()
       + Vector(code->xmin(), code->ymin()));

    cropped->on() = true;

    m_timed.insert (std::make_pair (m_current_time + Config::button_click_duration,
                                    C::make_handle<C::Signal>
                                    ("code:stop_flashing")));

    if (code->failure())
    {
      code->reset();
      emit ("code:play_failure");
    }
    else if (code->success())
    {
      code->reset();
      emit ("code:play_success");
      m_timed.insert (std::make_pair (m_current_time + Config::button_click_duration,
                                      C::make_handle<C::Signal>
                                      ("code:quit")));
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
    auto window = get<C::Image>("Game:window");
    window->on() = false;
    code->reset();
    status->pop();

    m_current_action = get<C::Action>
      (get<C::Code>("Game:code")->entity() + ":action");
    m_next_step = 0;
  }

  if (auto new_room_origin = request<C::String>("Game:new_room_origin"))
  {
    m_current_action = get<C::Action>(new_room_origin->value() + ":action");
    m_next_step = 0;
    remove ("Game:new_room_origin");
  }

  auto action = request<C::Action>("Character:action");
  if (action && action != m_current_action)
  {
    clear_timed(false);
    m_current_action = action;
    m_next_step = 0;
    remove ("Character:action", true);
  }

  if (m_current_action)
  {
    if (!still_waiting || m_timed.empty())
    {
      if (m_next_step == m_current_action->size())
      {
        m_current_action = nullptr;
      }
      else
        while (m_next_step != m_current_action->size())
        {
          const C::Action::Step& s = (*m_current_action)[m_next_step ++];

          if (!apply_step(s))
            break;
        }
    }
  }

  update_camera();
  update_debug_info (get<C::Debug>(GAME__DEBUG));
}

void Logic::run_cutscene()
{
  auto cutscene = get<C::Cutscene>("Game:cutscene");
  double current_time
      = cutscene->current_time (m_current_time, get<C::Status>(GAME__STATUS)->value() == PAUSED);
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
      get<C::Status>(GAME__STATUS)->pop();
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
      img->set_scale(zoom);
      set<C::Position>(img->entity() + ":position", Point(x,y));

    }
    else if (auto music = request<C::Music>(el.id))
    {
      set<C::Variable>("Game:music", get<C::Music>(el.id));
      emit ("Music:start");
      el.active = false;
    }
    else
    {
      check (false, "Can't find cutscene element " + el.id);
    }
  }
}

void Logic::clear_timed(bool action_goto)
{
  std::set<Timed_handle> new_timed_handle;

  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
      continue;
    else if (action_goto && th.second->id().find("Comment_") == 0) // keep dialogs when moving
    {
      new_timed_handle.insert(th);
    }
    else
    {
      if (C::cast<C::Signal>(th.second))
        set (th.second);
      else
        remove (th.second->id());
    }
  m_timed.swap(new_timed_handle);
}

bool Logic::compute_path_from_target (C::Position_handle target,
                                      std::string id)
{
  auto ground_map = get<C::Ground_map>("Background:ground_map");

  if (id == "")
    id = get<C::String>("Player:name")->value();
  auto position = get<C::Position>(id + "_body:position");

  Point origin = position->value();
  Point t = target->value();

  debug("Target = " + to_string(t));

  if (target->component() != "view")
  {
    debug ("Camera position = " + std::to_string(get<C::Double>(CAMERA__POSITION)->value()));
    t = t + Vector (get<C::Double>(CAMERA__POSITION)->value(), 0);
  }

  debug("Computing path from " + to_string(origin) + " to " + to_string(t));
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

void Logic::update_camera()
{
  if (auto i = request<C::Double>("Shake:intensity"))
  {
    double begin = get<C::Double>("Shake:begin")->value();
    double end = get<C::Double>("Shake:end")->value();
    double intensity = i->value();
    double x_start = get<C::Double>("Camera:saved_position")->value();

    double current_intensity = intensity * (end - m_current_time) / (end - begin);

    constexpr double period = 0.02;

    double shift = std::sin ((m_current_time - begin) / period);

    get<C::Double>(CAMERA__POSITION)->set (x_start + shift * current_intensity);
  }
  else
  {
    auto position = get<C::Double>(CAMERA__POSITION);
    auto target = get<C::Double>("Camera:target");

    double dir = target->value() - position->value();
    dir *= Config::camera_speed;
    position->set (position->value() + dir);
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
    auto dbg_pos = set<C::Position>("Debug:position", Point(0,0));
  }
  else
  {
    auto dbg_img = request<C::Image> ("Debug:image");
    if (dbg_img)
      dbg_img->on() = false;

  }

}

bool Logic::apply_step (C::Action::Step s)
{
  check (m_dispatcher.find(s.function()) != m_dispatcher.end(),
         s.function() + " is not a valid function");
  return m_dispatcher[s.function()](s.args());
}

bool Logic::function_add (const std::vector<std::string>& args)
{
  check (args.size() == 2, "function_add() takes 2 arguments");
  std::string id = args[0];
  int diff = to_int(args[1]);

  auto integer = get<C::Int>(id + ":value");
  integer->set (integer->value() + diff);

  auto action = request<C::Action>(id + ":" + std::to_string(integer->value()));
  if (!action)
    action = get<C::Action>(id + ":default");

  for (std::size_t i = 0; i < action->size(); ++ i)
    apply_step ((*action)[i]);

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
    emit ("Music:fade");
    m_timed.insert (std::make_pair (m_current_time + duration, begin));
    m_timed.insert (std::make_pair (m_current_time + duration, end));
    m_timed.insert (std::make_pair (m_current_time + duration, out));
  }
  else if (option == "shake")
  {
    check (args.size() == 3, "function_camera(shake) takes 2 arguments");
    double intensity  = to_double(args[1]);
    double duration = to_double(args[2]);
    auto begin = set<C::Double> ("Shake:begin", m_current_time);
    auto end = set<C::Double> ("Shake:end", m_current_time + duration);
    auto intens = set<C::Double> ("Shake:intensity", intensity);
    auto camera = set<C::Double> ("Camera:saved_position", get<C::Double>(CAMERA__POSITION)->value());
    m_timed.insert (std::make_pair (m_current_time + duration, begin));
    m_timed.insert (std::make_pair (m_current_time + duration, end));
    m_timed.insert (std::make_pair (m_current_time + duration, intens));
    m_timed.insert (std::make_pair (m_current_time + duration, camera));

  }
  else if (option == "target")
  {
    check (args.size() == 2, "function_camera(target) takes 1 arguments");
    int position = to_int(args[1]);
    get<C::Double>("Camera:target")->set (position);
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
    get<C::Status>(GAME__STATUS)->push(LOCKED);
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
    get<C::Status>(GAME__STATUS)->pop();
    if (dialog->line().first != "")
    {
      m_current_action = get<C::Action>(dialog->line().first + ":action");
      m_next_step = 0;
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

    get<C::Status>(GAME__STATUS)->push(DIALOG_CHOICE);
    auto choices = set<C::Vector<std::string> >("Dialog:choices");
    dialog->get_choices (*choices);
    action->add ("dialog", { args[0], "continue" });

    // Keep track in case player saves and reload there
    set<C::String>("Game:current_dialog", args[0]);
  }

  if (action->size() != 0)
  {
    m_current_action = action;
    m_next_step = 0;
  }
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
    id = get<C::String>("Player:name")->value();
    args = init_args;
  }

  if (args.size() == 2)
  {
    if (compute_path_from_target
        (C::make_handle<C::Position>("Goto:view", Point (to_int(args[0]), to_int(args[1]))),
         id))
      m_timed.insert (std::make_pair (0, get<C::Path>(id + ":path")));
  }
  else
  {
    std::string target
        = (args.empty() ? m_current_action->target_entity() : args[0]);
    debug ("Action_goto " + target);

    auto position = request<C::Position>(target + ":view");
    if (compute_path_from_target(position, id))
      m_timed.insert (std::make_pair (0, get<C::Path>(id + ":path")));
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
    target = m_current_action->target_entity();

  debug ("Action_look " + target);
  const std::string& id = get<C::String>("Player:name")->value();

  if (target == "default" || !request<C::Position>(target + ":position"))
    set<C::Position>(id + ":lookat",
                                       get<C::Position>(CURSOR__POSITION)->value());
  else
  {
    auto state = request<C::String>(target + ":state");
    if (!state || state->value() != "inventory")
      set<C::Position>(id + ":lookat",
                                         get<C::Position>(target + ":position")->value());
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
      const std::string& character = get<C::String>("Player:name")->value();

      double duration = to_double(args[2]);
      set<C::String>(character + ":start_animation", target);

      m_timed.insert (std::make_pair (m_current_time + duration,
                                      C::make_handle<C::Signal>
                                      (character + ":stop_animation")));
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

        m_timed.insert (std::make_pair (end_time, C::make_handle<C::Signal>("Dummy:event")));
      }
    }
  }
  else if (option == "sound")
    emit ("play_sound:" + target);
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

    get<C::Position>(target + ":position")->set (Point(x, y));
    get<C::Image>(target + ":image")->z() = z;
  }
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
      get<C::Position>(target + ":position")->absolute() = false;
    }

    current_state->set (state);
    if (state == "inventory")
    {
      get<C::Inventory>("Game:inventory")->add(target);
      auto img
        = get<C::Image>(target + ":image");
      img->set_relative_origin(0.5, 0.5);
      img->z() = Config::inventory_back_depth;
      img->on() = false;
    }
  }
  else if (option == "visible")
  {
    if (auto boolean = request<C::Boolean>(target + ":visible"))
      emit (target + ":set_visible");
    else
    {
      auto image = get<C::Image>(target + ":image");
      image->on() = true;

      set<C::Variable>("Game:window", image);

      auto code = request<C::Code>(target + ":code");
      if (code)
      {
        get<C::Status>(GAME__STATUS)->push (IN_CODE);
        set<C::Variable>("Game:code", code);
      }
      else
        get<C::Status>(GAME__STATUS)->push (IN_WINDOW);
    }
  }
  else if (option == "hidden")
    emit (target + ":set_hidden");

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
    get<C::Status>(GAME__STATUS)->push(LOCKED);
  else if (option == "trigger")
  {
    std::string id = args[1];
    m_current_action = get<C::Action>(id + ":action");
    m_next_step = 0;
  }
  else if (option == "menu")
  {
    set<C::String>("Game:triggered_menu", args[1]);
    emit("Show:menu");
  }
  else if (option == "unlock")
    get<C::Status>(GAME__STATUS)->pop();
  else if (option == "wait")
  {
    if (args.size() == 2)
    {
      double time = to_double(args[1]);
      m_timed.insert (std::make_pair (m_current_time + time, C::make_handle<C::Base>("wait")));
    }
    return false;
  }
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
    id = get<C::String>("Player:name")->value();
    text = args[0];
  }
  else
  {
    check (args.size() == 2, "\"comment\" expects 1 or 2 arguments ("
           + std::to_string(args.size()) + "given)");
    id = args[0];
    text = args[1];
  }

  std::vector<C::Image_handle> dialog;
  create_dialog (id, text, dialog);

  int nb_char = int(text.size());
  double nb_seconds_read
      = get<C::Double>("Text:dialog_speed")->value()
      * (Config::min_reading_time + nb_char * Config::char_spoken_time);
  double nb_seconds_lips_moving = nb_char * Config::char_spoken_time;

  int y = 100;
  int x = int(get<C::Position>(id + "_body:position")->value().x()
              - get<C::Double>(CAMERA__POSITION)->value());

  for (auto img : dialog)
    if (x + 0.75 * img->width() / 2 > int(0.95 * Config::world_width))
      x = int(0.95 * Config::world_width - 0.75 * img->width() / 2);
    else if (x - 0.75 * img->width() / 2 < int(0.1 * Config::world_width))
      x = int(0.1 * Config::world_width + 0.75 * img->width() / 2);


  for (auto img : dialog)
  {
    auto pos = set<C::Position> (img->entity() + ":position", Point(x,y));
    y += img->height() * 1.1 * 0.75;

    m_timed.insert (std::make_pair (m_current_time + std::max(1., nb_seconds_read), img));
    m_timed.insert (std::make_pair (m_current_time + std::max(1., nb_seconds_read), pos));
  }

  emit (id + ":start_talking");

  m_timed.insert (std::make_pair (m_current_time + nb_seconds_lips_moving,
                                  C::make_handle<C::Signal>
                                  (id + ":stop_talking")));
  return true;
}

void Logic::create_dialog (const std::string& character,
                           const std::string& text,
                           std::vector<C::Image_handle>& dialog)
{
  static const int width_max = int(0.6 * Config::world_width);

  auto interface_font = get<C::Font> ("Interface:font");
  const std::string& color = get<C::String> (character + ":color")->value();

  auto img
    = set<C::Image> ("Comment:image",
                                       interface_font,
                                       color,
                                       text, true);
  img->set_scale(0.75);
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
                                           interface_font,
                                           color,
                                           std::string(text.begin() + std::ptrdiff_t(begin),
                                                       text.begin() + std::ptrdiff_t(end)), true);
      img->z() = Config::inventory_over_depth;
      img->set_scale(0.75);
      img->set_relative_origin(0.5, 0.5);
      dialog.push_back (img);
    }
  }

}


} // namespace Sosage::System
