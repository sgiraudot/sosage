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
#include <Sosage/System/Logic.h>
#include <Sosage/Utils/error.h>
#include <Sosage/Utils/geometry.h>

#include <algorithm>
#include <vector>

namespace Sosage::System
{

Logic::Logic (Content& content)
  : m_content (content), m_current_time(0)
  , m_current_action (nullptr), m_next_step (0)
{
}

void Logic::run (const double& current_time)
{
  m_current_time = current_time;

  std::set<Timed_handle> new_timed_handle;
  
  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
    {
      auto saved_path = Component::cast<Component::Path>(th.second);
      auto current_path = m_content.request<Component::Path>("character:path");
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
    DBG_CERR << "Collision with " << collision->id() << std::endl;
    if (auto name = m_content.request<Component::String>(collision->entity() + ":name"))
    {
      if (m_content.get<Component::String> ("chosen_verb:text")->entity() == "verb_goto")
        compute_path_from_target(m_content.get<Component::Position>(collision->entity() + ":view"));
    }
    else
      compute_path_from_target(m_content.get<Component::Position>("cursor:position"));
    
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
      std::cerr << "Failure" << std::endl;
      code->reset();
      m_content.set<Component::Event>("code:play_failure");
    }
    else if (code->success())
    {
      std::cerr << "Success" << std::endl;
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
    m_content.get<Component::String>("game:status")->set ("idle");
      
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
          DBG_CERR << s.get(0) << std::endl;

          if (s.get(0) == "comment")
            action_comment (s);
          else if (s.get(0) == "goto")
            action_goto (m_current_action->entity());
          else if (s.get(0) == "look")
            action_look (m_current_action->entity());
          else if (s.get(0) == "move")
            action_move (s);
          else if (s.get(0) == "play")
            action_play (s);
          else if (s.get(0) == "pick_animation")
            action_pick_animation (s);
          else if (s.get(0) == "set_state")
            action_set_state (s);
          else if (s.get(0) == "lock")
            m_content.get<Component::String>("game:status")->set("locked");
          else if (s.get(0) == "unlock")
            m_content.get<Component::String>("game:status")->set("idle");
          else if (s.get(0) == "show")
            action_show (s);
          else if (s.get(0) == "sync")
            break;
        }
    }
  }

  update_debug_info (m_content.get<Component::Debug>("game:debug"));
  update_console (m_content.get<Component::Console>("game:console"));
}

bool Logic::exit()
{
  return (m_content.request<Component::Event>("game:exit") != Component::Handle());
}

bool Logic::paused()
{
  return m_content.get<Component::Boolean>("game:paused")->value();
}

void Logic::clear_timed(bool action_goto)
{
  if (action_goto)
    std::cerr << "GOTO" << std::endl;
  
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
  std::cerr << m_timed.size() << std::endl;
}

bool Logic::compute_path_from_target (Component::Position_handle target)
{
  auto ground_map = m_content.get<Component::Ground_map>("background:ground_map");

  auto position = m_content.get<Component::Position>("character_body:position");
      
  Point origin = position->value();

  std::vector<Point> path;
  ground_map->find_path (origin, target->value(), path);

  // Check if character is already at target
  if ((path.size() == 1) && (path[0] == origin))
    return false;
  
  m_content.set<Component::Path>("character:path", path);
  return true;
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

void Logic::update_console (Component::Console_handle console)
{
  if (console->value())
  {
    auto debug_font = m_content.get<Component::Font> ("debug:font");
    auto console_img = m_content.set<Component::Image> ("console:image",
                                                        debug_font, "FF0000",
                                                        console->console_str());
    auto console_pos = m_content.set<Component::Position>("console:position", Point(0,0));
  }
  else
  {
    auto console_img = m_content.request<Component::Image> ("console:image");
    if (console_img)
      console_img->on() = false;
  }

}

void Logic::action_comment (Component::Action::Step step)
{
  std::string text = step.get(1);

  std::vector<Component::Image_handle> dialog;
  create_dialog (text, dialog);
  
  int nb_char = int(text.size());
  double nb_seconds = nb_char / m_content.get<Component::Double>("text:char_per_second")->value();

  int y = 100;
  int x = m_content.get<Component::Position>("character_body:position")->value().x();

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

  m_content.set<Component::Event>("character:start_talking");

  DBG_CERR << nb_seconds << std::endl;
  m_timed.insert (std::make_pair (m_current_time + nb_seconds,
                                  Component::make_handle<Component::Event>
                                  ("character:stop_talking")));
}

void Logic::action_goto (const std::string& target)
{
  auto position = m_content.request<Component::Position>(target + ":view");
  if (compute_path_from_target(position))
    m_timed.insert (std::make_pair (0, m_content.get<Component::Path>("character:path")));
}

void Logic::action_look (const std::string& target)
{
  if (target == "default")
    m_content.set<Component::Position>("character:lookat",
                                       m_content.get<Component::Position>("cursor:position")->value());
  else if (m_content.get<Component::String>(target + ":state")->value() != "inventory")
    m_content.set<Component::Position>("character:lookat",
                                       m_content.get<Component::Position>(target + ":position")->value());
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
  animation->reset (true);
  animation->on() = true;
}

void Logic::action_pick_animation (Component::Action::Step step)
{
  double duration = step.get_double(1);
  m_content.set<Component::Event>("character:start_pick_animation");

  m_timed.insert (std::make_pair (m_current_time + duration,
                                  Component::make_handle<Component::Event>
                                  ("character:stop_pick_animation")));

}

void Logic::action_set_state (Component::Action::Step step)
{
  std::string target = step.get(1);
  std::string state = step.get(2);

  auto current_state = m_content.get<Component::String>(target + ":state");

  if (current_state->value() == "inventory")
    m_content.get<Component::Inventory>("game:inventory")->remove(target);
  
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

void Logic::action_show (Component::Action::Step step)
{
  std::string target = step.get(1);

  auto image = m_content.get<Component::Image>(target + ":image");
  image->on() = true;

  m_content.set<Component::Variable>("game:window", image);

  auto code = m_content.request<Component::Code>(target + ":code");
  if (code)
  {
    m_content.get<Component::String>("game:status")->set ("code");
    m_content.set<Component::Variable>("game:code", code);
  }
  else
    m_content.get<Component::String>("game:status")->set ("window");
}

void Logic::create_dialog (const std::string& text, std::vector<Component::Image_handle>& dialog)
{
  static const double scale = 0.75;
  static const int width_max = int(0.95 * Config::world_width);
  
  auto img
    = m_content.set<Component::Image> ("comment:image",
                                       m_content.get<Component::Font> ("interface:font"),
                                       m_content.get<Component::String> ("interface:color")->value(),
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
    std::vector<std::size_t> cuts (nb_imgs + 1);
    cuts.front() = 0;
    cuts.back() = text.size();
    
    for (int i = 1; i < cuts.size() - 1; ++ i)
    {
      std::size_t cut = std::size_t(i * (text.size() / double(nb_imgs)));
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
      auto img
        = m_content.set<Component::Image> ("comment_" + std::to_string(i) + ":image",
                                           m_content.get<Component::Font> ("interface:font"),
                                           m_content.get<Component::String> ("interface:color")->value(),
                                           std::string(text.begin() + cuts[i],
                                                       text.begin() + cuts[i+1]), true);
      img->z() = Config::inventory_over_depth;
      img->set_scale(0.75);
      img->set_relative_origin(0.5, 0.5);
      dialog.push_back (img);
    }
  }

}


} // namespace Sosage::System
