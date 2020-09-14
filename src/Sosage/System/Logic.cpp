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
#include <Sosage/Component/Dialog.h>
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

namespace C = Component;

Logic::Logic (Content& content)
  : Base (content), m_current_time(0)
  , m_current_action (nullptr), m_next_step (0)
{
}

void Logic::run ()
{
  auto status = get<C::Status>(GAME__STATUS);
  if (status->value() == PAUSED || status->value() == LOADING
      || status->value() == DIALOG_CHOICE)
    return;

  double current_time = get<C::Double> (CLOCK__FRAME_TIME)->value();

  m_current_time = current_time;

  std::set<Timed_handle> new_timed_handle;

  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
    {
      const std::string& id = get<C::String>("player:name")->value();
      auto saved_path = C::cast<C::Path>(th.second);
      auto current_path = request<C::Path>(id + ":path");
      if (saved_path == current_path)
        new_timed_handle.insert(th);
    }
    else if (th.first <= m_current_time)
    {
      if (C::cast<C::Event>(th.second))
        set (th.second);
      else
        remove (th.second->id());
    }
    else
      new_timed_handle.insert(th);
  m_timed.swap(new_timed_handle);

  auto collision = request<C::Image> ("cursor:target");
  auto clicked = request<C::Event>("cursor:clicked");
  if (clicked && collision)
  {
    if (auto name = request<C::String>(collision->entity() + ":name"))
    {
      if (get<C::String> ("chosen_verb:text")->entity() == "verb_goto")
        compute_path_from_target(get<C::Position>(collision->entity() + ":view"));
    }
    else
      compute_path_from_target(get<C::Position>(CURSOR__POSITION));

    remove("cursor:clicked");
    remove("cursor:target");

    // Cancel current action
    clear_timed(true);
    m_current_action = nullptr;
  }

  if (auto code_clicked = request<C::Event>("code:button_clicked"))
  {
    auto code = get<C::Code>("game:code");
    auto window = get<C::Image>("game:window");
    auto cropped
      = get<C::Cropped>(window->entity() + "_button:image");

    cropped->crop (code->xmin(), code->xmax(), code->ymin(), code->ymax());
    set<C::Position>
      (window->entity() + "_button:position",
       get<C::Position>(window->entity() + ":position")->value()
       + Vector(code->xmin(), code->ymin()));

    cropped->on() = true;

    m_timed.insert (std::make_pair (m_current_time + Config::button_click_duration,
                                    C::make_handle<C::Event>
                                    ("code:stop_flashing")));

    if (code->failure())
    {
      code->reset();
      set<C::Event>("code:play_failure");
    }
    else if (code->success())
    {
      code->reset();
      set<C::Event>("code:play_success");
      m_timed.insert (std::make_pair (m_current_time + Config::button_click_duration,
                                      C::make_handle<C::Event>
                                      ("code:quit")));
    }
    else
      set<C::Event>("code:play_click");

    remove("code:button_clicked");
  }

  if (auto stop_flashing = request<C::Event>("code:stop_flashing"))
  {
    auto window = get<C::Image>("game:window");
    auto cropped
      = get<C::Cropped>(window->entity() + "_button:image");
    cropped->on() = false;
    remove("code:stop_flashing");
  }

  if (auto code_quit = request<C::Event>("code:quit"))
  {
    auto code = get<C::Code>("game:code");
    auto window = get<C::Image>("game:window");
    window->on() = false;
    code->reset();
    status->pop();

    m_current_action = get<C::Action>
      (get<C::Code>("game:code")->entity() + ":action");
    m_next_step = 0;

    remove("code:quit");
  }

  auto action = request<C::Action>("character:action");
  if (action && action != m_current_action)
  {
    clear_timed(false);
    m_current_action = action;
    m_next_step = 0;
    remove ("character:action", true);
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
          const C::Action::Step& s = (*m_current_action)[m_next_step ++];

          if (!apply_step(s))
            break;
        }
    }
  }

  update_camera();
  update_debug_info (get<C::Debug>(GAME__DEBUG));
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
      if (C::cast<C::Event>(th.second))
        set (th.second);
      else
        remove (th.second->id());
    }
  m_timed.swap(new_timed_handle);
}

bool Logic::compute_path_from_target (C::Position_handle target)
{
  auto ground_map = get<C::Ground_map>("background:ground_map");

  const std::string& id = get<C::String>("player:name")->value();
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

  set<C::Path>(id + ":path", path);
  return true;
}

void Logic::update_camera()
{
  auto position = get<C::Double>(CAMERA__POSITION);
  auto target = get<C::Double>("camera:target");

  double dir = target->value() - position->value();
  dir *= Config::camera_speed;
  position->set (position->value() + dir);
}

void Logic::update_debug_info (C::Debug_handle debug_info)
{
  if (debug_info->value())
  {
    auto debug_font = get<C::Font> ("debug:font");
    auto dbg_img = set<C::Image> ("debug:image",
                                                    debug_font, "FF0000",
                                                    debug_info->debug_str());
    auto dbg_pos = set<C::Position>("debug:position", Point(0,0));
  }
  else
  {
    auto dbg_img = request<C::Image> ("debug:image");
    if (dbg_img)
      dbg_img->on() = false;

  }

}

bool Logic::apply_step (C::Action::Step s)
{
  if (s.get(0) == "animate")
    action_animate (s);
  else if (s.get(0) == "comment")
    action_comment (s);
  else if (s.get(0) == "dialog")
  {
    action_dialog (s);
    return false; // action dialog replaces current action
  }
  else if (s.get(0) == "goto")
    action_goto (m_current_action->target_entity());
  else if (s.get(0) == "increment")
    action_modify (s.get(1), 1);
  else if (s.get(0) == "decrement")
    action_modify (s.get(1), -1);
  else if (s.get(0) == "load")
    action_load (s);
  else if (s.get(0) == "look")
    action_look (m_current_action->target_entity());
  else if (s.get(0) == "move")
    action_move (s);
  else if (s.get(0) == "play")
    action_play (s);
  else if (s.get(0) == "lock")
    get<C::Status>(GAME__STATUS)->push(LOCKED);
  else if (s.get(0) == "set_state")
    action_set_state (s);
  else if (s.get(0) == "set_coordinates")
    action_set_coordinates (s);
  else if (s.get(0) == "show")
    action_show (s);
  else if (s.get(0) == "sync")
    return false;
  else if (s.get(0) == "unlock")
    get<C::Status>(GAME__STATUS)->pop();

  return true;
}

void Logic::action_comment (C::Action::Step step)
{
  std::string id;
  std::string text;

  if (step.size() == 2)
  {
    id = get<C::String>("player:name")->value();
    text = step.get(1);
  }
  else
  {
    check (step.size() == 3, "\"comment\" expects 2 or 3 arguments ("
           + std::to_string(step.size()-1) + "given)");
    id = step.get(1);
    text = step.get(2);
  }

  std::vector<C::Image_handle> dialog;
  create_dialog (id, text, dialog);

  int nb_char = int(text.size());
  double nb_seconds = nb_char / get<C::Double>("text:char_per_second")->value();

  int y = 100;
  int x = get<C::Position>(id + "_body:position")->value().x()
          - get<C::Double>(CAMERA__POSITION)->value();

  for (auto img : dialog)
    if (x + 0.75 * img->width() / 2 > int(0.95 * Config::world_width))
      x = int(0.95 * Config::world_width) - 0.75 * img->width() / 2;
    else if (x - 0.75 * img->width() / 2 < int(0.1 * Config::world_width))
      x = int(0.1 * Config::world_width) + 0.75 * img->width() / 2;


  for (auto img : dialog)
  {
    auto pos = set<C::Position> (img->entity() + ":position", Point(x,y));
    y += img->height() * 1.1 * 0.75;

    m_timed.insert (std::make_pair (m_current_time + std::max(1., nb_seconds), img));
    m_timed.insert (std::make_pair (m_current_time + std::max(1., nb_seconds), pos));
  }

  set<C::Event>(id + ":start_talking");

  m_timed.insert (std::make_pair (m_current_time + nb_seconds,
                                  C::make_handle<C::Event>
                                  (id + ":stop_talking")));
}

void Logic::action_dialog (C::Action::Step step)
{
  auto dialog = get<C::Dialog>(step.get(1) + ":dialog");

  auto action = C::make_handle<C::Action>("dialog:action");

  if (step.size() == 2)
  {
    get<C::Status>(GAME__STATUS)->push(LOCKED);
    dialog->init();
  }
  else if (auto choice = request<C::Int>("dialog:choice"))
  {
    action->add ({ "comment",
                   get<C::Vector<std::string> >("dialog:choices")
                   ->value()[std::size_t(choice->value())] });
    action->add ({ "sync" });
    dialog->next(choice->value());
    remove("dialog:choice");
  }
  else
    dialog->next();

  if (dialog->is_over())
    get<C::Status>(GAME__STATUS)->pop();
  else if (dialog->is_line())
  {
    std::string character;
    std::string line;
    std::tie (character, line) = dialog->line();
    action->add ({ "comment", character, line });
    action->add ({ "sync" });
    action->add ({ "dialog", step.get(1), "continue" });
  }
  else
  {
    get<C::Status>(GAME__STATUS)->push(DIALOG_CHOICE);
    auto choices = set<C::Vector<std::string> >("dialog:choices");
    dialog->get_choices (*choices);
    action->add ({ "dialog", step.get(1), "continue" });
  }

  if (action->size() != 0)
  {
    m_current_action = action;
    m_next_step = 0;
  }
}

void Logic::action_goto (const std::string& target)
{
  debug ("action_goto " + target);
  const std::string& id = get<C::String>("player:name")->value();

  auto position = request<C::Position>(target + ":view");
  if (compute_path_from_target(position))
    m_timed.insert (std::make_pair (0, get<C::Path>(id + ":path")));
}

void Logic::action_load (C::Action::Step step)
{
  set<C::String>("game:new_room", step.get(1));
  set<C::String>("game:new_room_origin", step.get(2));
}

void Logic::action_look (const std::string& target)
{
  debug ("action_look " + target);
  const std::string& id = get<C::String>("player:name")->value();

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
}

void Logic::action_modify (const std::string& id, int diff)
{
  auto integer = get<C::Int>(id + ":value");
  integer->set (integer->value() + diff);

  auto action = request<C::Action>(id + ":" + std::to_string(integer->value()));
  if (!action)
    action = get<C::Action>(id + ":default");

  for (std::size_t i = 0; i < action->size(); ++ i)
    apply_step ((*action)[i]);
}


void Logic::action_move (C::Action::Step step)
{
  std::string target = step.get(1);
  int x = step.get_int(2);
  int y = step.get_int(3);
  int z = step.get_int(4);

  set<C::Position>(target + ":position", Point(x, y));
  get<C::Image>(target + ":image")->z() = z;
}

void Logic::action_play (C::Action::Step step)
{
  std::string target = step.get(1);

  auto animation = get<C::Animation>(target + ":image");
  animation->reset (true, 1);
  animation->on() = true;
}

void Logic::action_animate (C::Action::Step step)
{
  const std::string& character = get<C::String>("player:name")->value();

  std::string id = step.get(1);
  double duration = step.get_double(2);
  set<C::String>(character + ":start_animation", id);

  m_timed.insert (std::make_pair (m_current_time + duration,
                                  C::make_handle<C::Event>
                                  (character + ":stop_animation")));

}

void Logic::action_set_state (C::Action::Step step)
{
  std::string target = step.get(1);

  auto current_state = get<C::String>(target + ":state");

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
    get<C::Inventory>("game:inventory")->remove(target);
    get<C::Position>(target + ":position")->absolute() = false;
  }

  current_state->set (state);
  if (state == "inventory")
  {
    get<C::Inventory>("game:inventory")->add(target);
    auto img
      = get<C::Image>(target + ":image");
    img->set_relative_origin(0.5, 0.5);
    img->z() = Config::inventory_back_depth;
    img->on() = false;
  }
}

void Logic::action_set_coordinates (C::Action::Step step)
{
  std::string target = step.get(1);
  int x = step.get_int(2);
  int y = step.get_int(3);
  int z = step.get_int(4);

  get<C::Position>(target + ":position")->set (Point(x, y));
  get<C::Image>(target + ":image")->z() = z;
}

void Logic::action_show (C::Action::Step step)
{
  std::string target = step.get(1);

  auto image = get<C::Image>(target + ":image");
  image->on() = true;

  set<C::Variable>("game:window", image);

  auto code = request<C::Code>(target + ":code");
  if (code)
  {
    get<C::Status>(GAME__STATUS)->push (IN_CODE);
    set<C::Variable>("game:code", code);
  }
  else
    get<C::Status>(GAME__STATUS)->push (IN_WINDOW);
}

void Logic::create_dialog (const std::string& character,
                           const std::string& text,
                           std::vector<C::Image_handle>& dialog)
{
  static const int width_max = int(0.95 * Config::world_width);

  auto interface_font = get<C::Font> ("interface:font");
  const std::string& color = get<C::String> (character + ":color")->value();

  auto img
    = set<C::Image> ("comment:image",
                                       interface_font,
                                       color,
                                       text, true);
  img->set_scale(0.75);
  img->set_relative_origin(0.5, 0.5);

  if (img->width() <= width_max)
    dialog.push_back (img);
  else
  {
    remove("comment:image");
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
        = set<C::Image> ("comment_" + std::to_string(i) + ":image",
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
