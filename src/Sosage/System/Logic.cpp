#include <Sosage/Component/Action.h>
#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Code.h>
#include <Sosage/Component/Condition.h>
#include <Sosage/Component/Event.h>
#include <Sosage/Component/Ground_map.h>
#include <Sosage/Component/Font.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Inventory.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
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

  std::vector<Timed_handle> new_timed_handle;
  
  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
    {
      Component::Path_handle saved_path
        = Component::cast<Component::Path>(th.second);
      Component::Path_handle current_path
        = m_content.request<Component::Path>("character:path");
      if (saved_path == current_path)
        new_timed_handle.push_back(th);
    }
    else if (th.first <= m_current_time)
    {
      if (Component::cast<Component::Event>(th.second))
        m_content.set (th.second);
      else
        m_content.remove (th.second->id());
    }
    else
      new_timed_handle.push_back(th);
  m_timed.swap(new_timed_handle);
  
  Component::Image_handle collision = m_content.request<Component::Image> ("cursor:target");
  Component::Event_handle clicked
    = m_content.request<Component::Event>("cursor:clicked");
  if (clicked && collision)
  {
    DBG_CERR << "Collision with " << collision->id() << std::endl;
    if (Component::Text_handle name = m_content.request<Component::Text>(collision->entity() + ":name"))
    {
      if (m_content.get<Component::Text> ("chosen_verb:text")->entity() == "verb_goto")
        compute_path_from_target(m_content.get<Component::Position>(collision->entity() + ":view"));
    }
    else
      compute_path_from_target(m_content.get<Component::Position>("cursor:position"));
    
    m_content.remove("cursor:clicked");
    m_content.remove("cursor:target");

    // Cancel current action
    clear_timed();
    if (m_current_action != nullptr)
      m_content.remove ("character:action");
    m_current_action = nullptr;
  }

  if (Component::Event_handle code_success = m_content.request<Component::Event>("code:success"))
  {
    m_current_action = m_content.get<Component::Action>
      (m_content.get<Component::Code>("game:code")->entity() + ":action");
    m_next_step = 0;
    m_content.remove ("code:success");
  }
   
  Component::Action_handle action
    = m_content.request<Component::Action>("character:action");
  if (action && action != m_current_action)
  {
    m_current_action = action;
    m_next_step = 0;
  }

  if (m_current_action)
  {
    if (m_timed.empty())
    {
      if (m_next_step == m_current_action->size())
      {
        m_content.remove ("character:action", true);
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
          else if (s.get(0) == "pick_animation")
            action_pick_animation (s);
          else if (s.get(0) == "set_state")
            action_set_state (s);
          else if (s.get(0) == "lock")
            m_content.get<Component::State>("game:status")->set("locked");
          else if (s.get(0) == "unlock")
            m_content.get<Component::State>("game:status")->set("idle");
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

void Logic::clear_timed()
{
  std::vector<Timed_handle> new_timed_handle;
  
  for (const Timed_handle& th : m_timed)
    if (th.first == 0) // special case for Path
      continue;
    else if (th.second->id().find("comment_") == 0) // keep dialogs
    {
      new_timed_handle.push_back(th);
      DBG_CERR << "Keep " << th.second->id();
    }
    else if (!Component::cast<Component::Event>(th.second))
    {
      DBG_CERR << "Remove " << th.second->id();
      m_content.remove (th.second->id());
    }
  m_timed.swap(new_timed_handle);
}

bool Logic::compute_path_from_target (Component::Position_handle target)
{
  Component::Ground_map_handle ground_map
    = m_content.get<Component::Ground_map>("background:ground_map");

  Component::Position_handle position
    = m_content.get<Component::Position>("character_body:position");
      
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
    Component::Font_handle debug_font
      = m_content.get<Component::Font> ("debug:font");
    Component::Image_handle dbg_img
      = m_content.set<Component::Image> ("debug:image",
                                         debug_font, "FF0000",
                                         debug_info->debug_str());
    Component::Position_handle dbg_pos
      = m_content.set<Component::Position>("debug:position", Point(0,0));
  }
  else
  {
    Component::Image_handle dbg_img = m_content.request<Component::Image> ("debug:image");
    if (dbg_img)
      dbg_img->on() = false;
    
  }

}

void Logic::update_console (Component::Console_handle console)
{
  if (console->value())
  {
    Component::Font_handle debug_font
      = m_content.get<Component::Font> ("debug:font");
    Component::Image_handle console_img
      = m_content.set<Component::Image> ("console:image",
                                         debug_font, "FF0000",
                                         console->console_str());
    Component::Position_handle console_pos
      = m_content.set<Component::Position>("console:position", Point(0,0));
  }
  else
  {
    Component::Image_handle console_img = m_content.request<Component::Image> ("console:image");
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
  double nb_seconds = nb_char / double(config().characters_per_second);

  int y = 100;
  int x = m_content.get<Component::Position>("character_body:position")->value().x();

  for (Component::Image_handle img : dialog)
    if (x + 0.75 * img->width() / 2 > int(0.95 * Sosage::world_width))
      x = int(0.95 * Sosage::world_width) - 0.75 * img->width() / 2;
    else if (x - 0.75 * img->width() / 2 < int(0.1 * Sosage::world_width))
      x = int(0.1 * Sosage::world_width) + 0.75 * img->width() / 2;

    
  for (Component::Image_handle img : dialog)
  {
    Component::Position_handle pos
      = m_content.set<Component::Position> (img->entity() + ":position", Point(x,y));
    y += img->height() * 1.1 * 0.75;

    m_timed.push_back (std::make_pair (m_current_time + std::max(1., nb_seconds), img));
    m_timed.push_back (std::make_pair (m_current_time + std::max(1., nb_seconds), pos));
  }

  m_content.set<Component::Event>("character:start_talking");

  DBG_CERR << nb_seconds << std::endl;
  m_timed.push_back (std::make_pair (m_current_time + nb_seconds,
                                     Component::make_handle<Component::Event>
                                     ("character:stop_talking")));
}

void Logic::action_goto (const std::string& target)
{
  Component::Position_handle position
    = m_content.request<Component::Position>(target + ":view");
  if (compute_path_from_target(position))
    m_timed.push_back (std::make_pair (0, m_content.get<Component::Path>("character:path")));
}

void Logic::action_look (const std::string& target)
{
  if (target == "default")
    m_content.set<Component::Position>("character:lookat",
                                       m_content.get<Component::Position>("cursor:position")->value());
  else if (m_content.get<Component::State>(target + ":state")->value() != "inventory")
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

void Logic::action_pick_animation (Component::Action::Step step)
{
  double duration = step.get_double(1);
  m_content.set<Component::Event>("character:start_pick_animation");

  m_timed.push_back (std::make_pair (m_current_time + duration,
                                     Component::make_handle<Component::Event>
                                     ("character:stop_pick_animation")));

}

void Logic::action_set_state (Component::Action::Step step)
{
  std::string target = step.get(1);
  std::string state = step.get(2);

  Component::State_handle current_state
    = m_content.get<Component::State>(target + ":state");

  if (current_state->value() == "inventory")
    m_content.get<Component::Inventory>("game:inventory")->remove(target);
  
  current_state->set (state);
  if (state == "inventory")
  {
    m_content.get<Component::Inventory>("game:inventory")->add(target);
    Component::Image_handle img
      = m_content.get<Component::Image>(target + ":image");
    img->set_relative_origin(0.5, 0.5);
    img->z() = Sosage::inventory_back_depth;
  }
}

void Logic::action_show (Component::Action::Step step)
{
  std::string target = step.get(1);

  Component::Image_handle image
    = m_content.get<Component::Image>(target + ":image");
  image->on() = true;

  m_content.set<Component::Variable>("game:window", image);

  Component::Code_handle code
    = m_content.request<Component::Code>(target + ":code");
  if (code)
  {
    m_content.get<Component::State>("game:status")->set ("code");
    m_content.set<Component::Variable>("game:code", code);
  }
  else
    m_content.get<Component::State>("game:status")->set ("window");
}

void Logic::create_dialog (const std::string& text, std::vector<Component::Image_handle>& dialog)
{
  static const double scale = 0.75;
  static const int width_max = int(0.95 * Sosage::world_width);
  
  Component::Image_handle img
    = m_content.set<Component::Image> ("comment:image",
                                       m_content.get<Component::Font> ("interface:font"),
                                       m_content.get<Component::Text> ("interface:color")->value(),
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
      Component::Image_handle img
        = m_content.set<Component::Image> ("comment_" + std::to_string(i) + ":image",
                                           m_content.get<Component::Font> ("interface:font"),
                                           m_content.get<Component::Text> ("interface:color")->value(),
                                           std::string(text.begin() + cuts[i],
                                                       text.begin() + cuts[i+1]), true);
      img->z() = Sosage::inventory_over_depth;
      img->set_scale(0.75);
      img->set_relative_origin(0.5, 0.5);
      dialog.push_back (img);
    }
  }

}


} // namespace Sosage::System
