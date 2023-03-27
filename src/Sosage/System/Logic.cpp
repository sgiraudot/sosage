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
#include <Sosage/Component/Cropped.h>
#include <Sosage/Component/Dialog.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
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
#include <Sosage/Utils/helpers.h>

#include <algorithm>
#include <fstream>
#include <functional>
#include <vector>

// this line creates a dispatcher, for example INIT_DISPATCHER(name) -> m_dispatcher["name"] = Logic::function_name
#define INIT_DISPATCHER(x) \
  m_dispatcher.insert (std::make_pair(#x, std::bind(&Logic::function_##x, this, std::placeholders::_1)))

namespace Sosage::System
{

namespace C = Component;

Logic::Logic (Content& content)
  : Base (content), m_current_time(0)
{
  INIT_DISPATCHER(add);
  INIT_DISPATCHER(camera);
  INIT_DISPATCHER(control);
  INIT_DISPATCHER(cutscene);
  INIT_DISPATCHER(emit);
  INIT_DISPATCHER(exit);
  INIT_DISPATCHER(fadein);
  INIT_DISPATCHER(fadeout);
  INIT_DISPATCHER(goto);
  INIT_DISPATCHER(hide);
  INIT_DISPATCHER(load);
  INIT_DISPATCHER(lock);
  INIT_DISPATCHER(look);
  INIT_DISPATCHER(loop);
  INIT_DISPATCHER(message);
  INIT_DISPATCHER(move);
  INIT_DISPATCHER(move60fps);
  INIT_DISPATCHER(pause);
  INIT_DISPATCHER(play);
  INIT_DISPATCHER(randomize);
  INIT_DISPATCHER(receive);
  INIT_DISPATCHER(rescale);
  INIT_DISPATCHER(rescale60fps);
  INIT_DISPATCHER(set);
  INIT_DISPATCHER(set12fps);
  INIT_DISPATCHER(shake);
  INIT_DISPATCHER(show);
  INIT_DISPATCHER(stop);
  INIT_DISPATCHER(talk);
  INIT_DISPATCHER(timer);
  INIT_DISPATCHER(trigger);
  INIT_DISPATCHER(unlock);
  INIT_DISPATCHER(wait);
  INIT_DISPATCHER(zoom);

  set<C::Action>("Logic", "action");

  set<C::String>("Hinter", "color", "FFFFFF");
  set<C::Functional_position>("Hinter", "position",
                              [&](const std::string&) -> Point
  {
    const std::string& player = value<C::String>("Player", "name");
    Vector diff ((is_looking_right(player) ? 50 : -50), 0);
    return value<C::Position>(player, "position") + diff;
  }, "");
}

void Logic::run ()
{
  SOSAGE_TIMER_START(System_Logic__run);
  SOSAGE_UPDATE_DBG_LOCATION("Logic::run()");

  m_current_time = value<C::Double> (CLOCK__TIME);
  update_debug_info (get<C::Debug>(GAME__DEBUG));
  if (status()->is (PAUSED, DIALOG_CHOICE, IN_MENU)
      || signal("Game", "reset"))
  {
    SOSAGE_TIMER_STOP(System_Logic__run);
    return;
  }

  bool in_new_room = false;
  if (signal("Game", "in_new_room"))
  {
    in_new_room = true;
    status()->pop();
  }

  bool skip_dialog = receive("Game", "skip_dialog");

  if (receive ("Cancel", "action"))
  {
    // Cancel current action
    if (auto action = request<C::Action>("Character", "action"))
    {
      debug << "Cancel action " << action->str() << std::endl;
      auto logic_action = get<C::Action>("Logic", "action");

      for (const auto& th : action->scheduled())
      {
        if (th.first == 0) // special case for Path and animations
        {
          remove (th.second->entity(), "nofollow", true);
          continue;
        }
        if (th.second->entity().find("Comment_") == 0) // keep dialogs when moving
          logic_action->schedule (th.first, th.second);
        else
        {
          if (C::cast<C::Signal>(th.second))
            set (th.second);
          else if (th.second->entity() != "wait")
            remove (th.second);
        }
      }

      action->stop();
      remove("Character", "action");
      const std::string& id = value<C::String>("Player", "name");
      remove(id , "path", true);
      remove(id, "speed_factor", true);
      emit(id, "stop_walking");
    }
  }

  if (auto console_action = request<C::String>("Test", "console_action"))
  {
    std::stringstream ss(console_action->value());
    std::string function;
    std::getline(ss, function, ' ');
    if (m_dispatcher.find(function) == m_dispatcher.end())
    {
      debug << "Function " << function << " not found" << std::endl;
    }
    else
    {
      std::vector<std::string> args(1);
      while (std::getline(ss, args.back(), ' '))
      {
        args.emplace_back();
      }
      args.pop_back();

      auto action = set<C::Action>("Console", "action");
      action->add (function, args);
      action->launch();
    }
    remove (console_action);
  }

  std::set<std::string> done;
  for (auto c : components("action"))
    if (auto a = C::cast<C::Action>(c))
      if (c->entity() != "Character")
      {
        a->update_scheduled
            ([&](const C::Action::Timed_handle& th) -> bool
        {
          if (th.first == 0) // special case for Path and animations
          {
            if (auto saved_path = C::cast<C::Path>(th.second))
            {
              auto current_path = request<C::Path>(saved_path->entity(), saved_path->component());
              if (saved_path == current_path)
                return true;
              remove (saved_path->entity(), "nofollow", true);
              return false;
            }
            // else
            auto anim = C::cast<C::Animation>(th.second);
            if (anim->is_last_frame())
              return false;
            return true;
          }
          if (th.first <= m_current_time)
          {
            if (C::cast<C::Signal>(th.second))
              set (th.second);
            else if (th.second->entity() != "wait")
            {
              debug << a->str() << " remove " << th.second->str() << std::endl;

              // avoid removing new dialog with an outdated timed dialog
              if (startswith(th.second->entity(), "Comment_"))
              {
                auto saved_dialog = C::cast<C::Image>(th.second);
                auto current_dialog = request<C::Image>(saved_dialog->entity(),
                                                        saved_dialog->component());
                if (saved_dialog == current_dialog)
                  remove (current_dialog);
              }
              else
                remove (th.second, true);
            }
            return false;
          }
          else if (skip_dialog)
          {
            if (contains(th.second->entity(), "Comment_"))
            {
              // comments might already have been removed
              remove (th.second, true);
              return false;
            }
            else if (th.second->component() == "stop_talking")
            {
              set (th.second);
              return false;
            }
          }
          return true;
        });
      }


  if (receive ("Cursor", "clicked"))
    compute_path_from_target(get<C::Position>(CURSOR__POSITION));

  if (receive ("Game", "test"))
  {

#if 0 // Uncomment to quickly test paths
    Point p (-0.5, 970);
    Vector v (0.562398475328108716, -0.629368298259299408);
    auto ground_map = get<C::Ground_map>("background", "ground_map");
    std::vector<Point> path;
    ground_map->find_path (p, v, path);
    try
    {
    }
    catch (std::runtime_error& e)
    {
      debug << e.what() << std::endl;
      //exit(1);
    }
    set<C::Absolute_position>("Debug_body", "position", p);
    set<C::Path>("Debug", "path", path);
#endif
  }

  if (receive ("Stick", "moved"))
  {
    auto direction = get<C::Simple<Vector>>(STICK__DIRECTION);
    const std::string& id = value<C::String>("Player", "name");
    if (direction->value() == Vector(0,0))
    {
      remove(id , "path", true);
      remove(id, "speed_factor", true);
      emit(id , "stop_walking");
    }
    else
    {
      auto position = get<C::Position>(id + "_body", "position");

      // Make direction "flatter" (45° becomes 30°, etc.)
      Vector dir = direction->value();
      dir = Vector (dir.x() * 1.22, dir.y() / 1.41);
      compute_path_from_direction(dir);
    }
  }

  if (receive ("code", "button_clicked"))
  {
    auto code = get<C::Code>("Game", "code");
    auto window = get<C::Image>("Game", "window");
    auto cropped
      = get<C::Cropped>(window->entity() + "_button", "image");

    cropped->crop (code->xmin(), code->xmax(), code->ymin(), code->ymax());
    set<C::Absolute_position>
      (window->entity() + "_button", "position",
       value<C::Position>(window->entity() , "position")
       + Vector(code->xmin(), code->ymin()));

    cropped->on() = true;

    get<C::Action>("Logic", "action")->schedule (m_current_time + Config::button_click_duration,
                                              C::make_handle<C::Signal>("code", "stop_flashing"));

    if (code->failure())
    {
      code->reset();
      emit ("code", "play_failure");
    }
    else if (code->success())
    {
      code->reset();
      emit ("code", "play_success");
      get<C::Action>("Logic", "action")->schedule (m_current_time + Config::button_click_duration,
                                                C::make_handle<C::Signal>("code", "quit"));
      status()->push(LOCKED);
    }
    else
      emit ("code", "play_click");
  }

  if (receive ("code", "cheat")) // For testing purposes...
  {
    debug << "CHEAT CODE RECEIVED" << std::endl;
    auto code = get<C::Code>("Game", "code");
    code->reset();
    emit ("code", "play_success");
    get<C::Action>("Logic", "action")->schedule (m_current_time + Config::button_click_duration,
                                              C::make_handle<C::Signal>("code", "quit"));
    status()->push(LOCKED);
  }

  if (receive ("code", "stop_flashing"))
  {
    auto window = get<C::Image>("Game", "window");
    auto cropped
      = get<C::Cropped>(window->entity() + "_button", "image");
    cropped->on() = false;
  }

  if (receive ("code", "quit"))
  {
    auto code = get<C::Code>("Game", "code");
    emit("Interface", "hide_window");
    code->reset();
    remove("Code_hover", "image", true);
    status()->pop();
    status()->pop();

    get<C::Action>(get<C::Code>("Game", "code")->entity() , "action")->launch();
  }

  if (!in_new_room)
    if (auto follower = request<C::String>("Follower", "name"))
      if (!request<C::Base>(follower->value(), "nofollow"))
        follow (follower->value());

  if (auto new_room_origin = request<C::String>("Game", "new_room_origin"))
  {
    get<C::Action>(new_room_origin->value() , "action")->launch();
    if (new_room_origin->value() == "Saved_game")
    {
      auto room_name = value<C::String>("Game", "current_room");
      if (auto action = request<C::Action>(room_name + "_save", "action"))
        action->launch();
    }
    remove ("Game", "new_room_origin");
  }

  if (auto triggered_action = request<C::Action>("Character", "triggered_action"))
  {
    if (auto action = request<C::Action>("Character", "action"))
    {
      debug << "Action " << action->entity() << " interrupted" << std::endl;
      for (const auto& th : action->scheduled())
      {
        if (th.first != 0) // special case for Path
        {
          if (C::cast<C::Signal>(th.second))
            set (th.second);
          else if (th.second->entity() != "wait")
            // comments might already have been removed
            remove (th.second->entity(), th.second->component(), true);
        }
      }
      action->stop();
    }
    triggered_action->launch();

    // Very specific event: if we're about to leave a room and we're
    // already pretty close to the target point, don't move at all
    // and immediately leave
    if (endswith(triggered_action->entity(), "_goto")
        && triggered_action->first_step().function() == "goto"
        && triggered_action->last_step().function() == "load")
    {
      auto pos = value<C::Position>(value<C::String>("Player", "name"), "position");
      auto target = value<C::Position>(triggered_action->target_entity(), "position");
      if (std::abs(pos.X() - target.X()) < Config::goto_active_zone_x
          && std::abs(pos.Y() - target.Y()) < Config::goto_active_zone_y)
      {
        debug << "Player is already close to target, leave room immediately" << std::endl;
        triggered_action->next_step();
      }
    }

    debug << "Action " << triggered_action->entity() << " launched" << std::endl;
    set<C::Variable>("Character", "action", triggered_action);
    remove ("Character", "triggered_action");
  }

  bool skip = false;
  if (status()->is(CUTSCENE))
  {
    skip = receive ("Game", "skip_cutscene");
    if (skip)
      status()->pop();
  }

  // Quick'n'dirty workaround for the cutscene camera
  // bug. I should rework the full wait/schedule/etc.
  // framework to be clean.
  if (skip)
    remove("Camera", "move60fps", true);

  for (auto c : components("action"))
    if (auto a = C::cast<C::Action>(c))
      if (c->entity() != "Character" && a->on())
          m_todo.push (a);

  while (!m_todo.empty())
  {
    auto a = m_todo.front();
    m_todo.pop();
    if (skip)
    {
      C::Action::Step s = a->next_step();
      a->reset_scheduled();
      while (s.function() != "unlock" && a->on())
        s = a->next_step();
      if (!a->on())
        continue;
    }
    if (!a->ready())
      continue;
    //        debug << "Applying steps of action " << a->id() << std::endl;
    do
    {
      if (!apply_next_step (a))
        break;
    }
    while (a->on());

    // Action might have changed state, let's transfer the scheduled
    // steps if that happens
    auto new_a = request<C::Action>(a->entity(), a->component());
    if (new_a && new_a != a && !a->scheduled().empty())
    {
      for (const auto& th : a->scheduled())
        new_a->schedule (th.first, th.second);
      a->reset_scheduled();
    }
  }

  SOSAGE_TIMER_STOP(System_Logic__run);
}

bool Logic::compute_path_from_target (C::Position_handle target,
                                      std::string id)
{
  bool is_player = false;
  if (id == "")
  {
    id = value<C::String>("Player", "name");
    is_player = true;
  }

  auto ground_map = get_ground_map(id);
  if (!ground_map)
    return false;

  auto position = get<C::Position>(id + "_body", "position");

  Point origin = position->value();
  Point t = target->value();

  //debug("Target = ", t);

  if (target->component() != "view" && !contains(target->entity(), "_body"))
    t = t + value<C::Absolute_position>(CAMERA__POSITION);


  //debug("Computing path from ", origin, " to ", t);
  std::vector<Point> path;
  ground_map->find_path (origin, t, path);

  // Check if character is already at target
  if (path.empty() || ((path.size() == 1) && (path[0] == origin)))
    return false;

  if (auto previous_path = request<C::Path>(id, "path"))
  {
    if (distance((*previous_path)[previous_path->size()-1], path[path.size()-1]) < 5)
    {
      if (is_player)
      {
        if (auto speed_factor = request<C::Double>(id, "speed_factor"))
        {
          if (speed_factor->value() < 2.2)
            speed_factor->set (speed_factor->value() + 0.25);
        }
        else
          set<C::Double>(id, "speed_factor", 1.25);
      }
      else
      {
        // Follower follows slightly less fast
        if (id == value<C::String>("Follower", "name", ""))
          if (auto speed_factor = request<C::Double>(value<C::String>("Player", "name"), "speed_factor"))
            set<C::Double>(id, "speed_factor", speed_factor->value() - 0.25);
      }
      return true;
    }
  }
  else
    remove(id, "speed_factor", true);

  auto p = set<C::Path>(id , "path", path);
  if ((*p)[0] == origin)
    p->current() ++;
  return true;
}

bool Logic::compute_path_from_direction (const Vector& direction)
{
  const std::string& id = value<C::String>("Player", "name");

  auto ground_map = get_ground_map(id);
  if (!ground_map)
    return false;

  auto position = get<C::Position>(id + "_body", "position");
  Point origin = position->value();

  std::vector<Point> path;

  ground_map->find_path (origin, direction, path);

  // Check if character is already at target
  if (path.empty() || ((path.size() == 1) && (path[0] == origin)))
    return false;

  auto p = set<C::Path>(id , "path", path);
  if ((*p)[0] == origin)
    p->current() ++;
  return true;
}

void Logic::follow (const std::string& follower)
{
  const std::string& player = value<C::String>("Player", "name");

  auto pos_player = get<C::Position>(player + "_body", "position");
  auto pos_follower = get<C::Position>(follower + "_body", "position");

  double dx = std::abs(pos_player->value().x() - pos_follower->value().x());
  double dy = std::abs(pos_player->value().y() - pos_follower->value().y());

  bool is_moving = value<C::Boolean>(follower , "walking");

  double z = get<C::Image>(player + "_body", "image")->z();

  int reach_x = Config::object_reach_x * z / Config::follow_factor;
  int reach_y = Config::object_reach_y * z / Config::follow_factor;
  int reach_hysteresis = 0;
  if (!is_moving)
    reach_hysteresis = Config::object_reach_hysteresis * z / Config::follow_factor;

  //std::cerr << reach_x << " " << reach_y << " " << reach_hysteresis << std::endl;

  // Object out of reach
  if (dx > reach_x + reach_hysteresis ||
      dy > reach_y + reach_hysteresis)
    compute_path_from_target (pos_player, follower);
  else
  {
    remove(follower , "path", true);
    remove(follower, "speed_factor", true);
    emit(follower , "stop_walking");
  }
}

void Logic::update_debug_info (C::Debug_handle debug_info)
{
  if (debug_info->value())
  {
    auto debug_font = get<C::Font> ("Debug", "font");
    auto dbg_img = set<C::Image> ("Debug", "image",
                                                    debug_font, "FFFFFF",
                                                    debug_info->debug_str());
    dbg_img->z() = Config::loading_depth;
    auto dbg_pos = set<C::Absolute_position>("Debug", "position", Point(0,0));
  }
  else
  {
    auto dbg_img = request<C::Image> ("Debug", "image");
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

bool Logic::subfunction_fade (bool fadein, double duration)
{
  auto fade = set<C::Tuple<double, double, bool>>("Camera", "fade",
                                                  m_current_time, m_current_time + duration, fadein);
  m_current_action->schedule (m_current_time + duration, C::make_handle<C::Base>("Fade", "dummy"));
  return true;
}

bool Logic::subfunction_trigger_dialog (const std::vector<std::string>& args)
{
  std::string id = args[0];
  bool is_continue = (args.size() > 1);
  auto dialog = get<C::Dialog>(id , "dialog");

  auto action = get<C::Action>("Logic", "action");
  action->clear();

  if (!is_continue)
  {
    if (status()->is(IN_INVENTORY))
      status()->pop();
    status()->push(LOCKED);
    if (auto pos = request<C::Int>("Saved_game", "dialog_position"))
    {
      dialog->init (pos->value());
      remove ("Saved_game", "dialog_position");
    }
    else
      dialog->init();
  }
  else if (auto choice = request<C::Int>("Dialog", "choice"))
  {
    if (dialog->is_displayed(choice->value()))
    {
      action->add ("talk",
      { get<C::Vector<std::string> >("Dialog", "choices")
        ->value()[std::size_t(choice->value())] });
      action->add ("wait", {});
    }
    std::string sig = dialog->next(choice->value());
    if (sig != "")
    {
      emit (sig, "signal");
      debug << "Emit " << sig << ":signal" << std::endl;
    }
    remove("Dialog", "choice");
  }
  else if (args[1] == "continue")
  {
    dialog->next();
  }

  if (dialog->is_over())
  {
    status()->pop();
    if (dialog->line().first != "")
    {
//      set<C::Variable>("Character", "triggered_action", get<C::Action>(dialog->line().first , "action"));
      auto end_action = get<C::Action>(dialog->line().first , "action");
      end_action->launch();
      m_todo.push(end_action);
      return false;
    }
  }
  else if (dialog->is_line())
  {
    std::string character;
    std::string line;
    std::tie (character, line) = dialog->line();
    const std::string& player = value<C::String>("Player", "name");
    if (player != character && character != "Hinter")
      action->add ("look", { character });
    if (auto follower = request<C::String>("Follower", "name"))
      if (follower->value() != character && character != "Hinter")
        action->add ("look", { follower->value(), character });

    action->add ("talk", { character, line });
    if (dialog->has_signal())
      action->add ("emit", {dialog->signal()});
    action->add ("wait", {});
    action->add ("trigger", { id, "continue" });

  }
  else if (action->size() != 0)
  {
    // If two dialog choices are chained, let's first allow the system
    // to display the first one before chosing the next one
    action->add ("trigger", { id, "no_next_continue" });
  }
  else
  {
    // Keep track in case player saves and reload there
    set<C::Int>("Game", "dialog_position", dialog->current());
    status()->push(DIALOG_CHOICE);
    auto choices = set<C::Vector<std::string> >("Dialog", "choices");
    dialog->get_choices (*choices, m_content);
    action->add ("trigger", { id, "continue" });

    // Keep track in case player saves and reload there
    set<C::String>("Game", "current_dialog", id);
  }

  if (action->size() != 0)
    //action->launch();
    set<C::Variable>("Character", "triggered_action", action);

  return false;
}

void Logic::create_dialog (const std::string& character,
                           const std::string& text,
                           std::vector<C::Image_handle>& dialog)
{
  // Clean up existing dialog that might still exist
  int idx = 0;
  while (remove("Comment_" + to_string(idx), "image", true))
    ++ idx;

  static const int width_max = int(0.6 * Config::world_width);

  double size_factor = 0.75 * (value<C::Int>("Dialog", "size") / double(Config::MEDIUM));
  auto font = get<C::Font> ("Dialog", "font");
  const std::string& color = value<C::String> (character , "color");
  int estimated_size = 25 * text.size() * size_factor;

  if (estimated_size < width_max)
  {
    auto img
        = set<C::Image> ("Comment_0", "image",
                         font,
                         color,
                         text, true);
    img->set_scale(size_factor);
    img->set_relative_origin(0.5, 0.5);
    img->z() = Config::dialog_depth;
    dialog.push_back (img);
  }
  else
  {
    int nb_imgs = 1 + (estimated_size / width_max);

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
        = set<C::Image> ("Comment_" + std::to_string(i) , "image",
                         font,
                         color,
                         std::string(text.begin() + std::ptrdiff_t(begin),
                                     text.begin() + std::ptrdiff_t(end)), true);
      img->z() = Config::dialog_depth;
      img->set_scale(size_factor);
      img->set_relative_origin(0.5, 0.5);
      dialog.push_back (img);
    }
  }

}

} // namespace Sosage::System
