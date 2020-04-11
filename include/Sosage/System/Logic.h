#ifndef SOSAGE_SYSTEM_LOGIC_H
#define SOSAGE_SYSTEM_LOGIC_H

#include <Sosage/Component/Action.h>
#include <Sosage/Component/Console.h>
#include <Sosage/Component/Debug.h>
#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Component/Text.h>
#include <Sosage/Content.h>

namespace Sosage::System
{

class Logic
{
private:

  typedef std::pair<double, Component::Handle> Timed_handle;
  std::vector<Timed_handle> m_timed;

  Content& m_content;
  double m_current_time;

  Component::Action_handle m_current_action;
  std::size_t m_next_step;

public:

  Logic (Content& content);

  void run(const double& current_time);
  bool exit();
  bool paused();

private:
  void clear_timed();
  
  bool compute_path_from_target (Component::Position_handle target);
  void update_debug_info (Component::Debug_handle debug_info);
  void update_console (Component::Console_handle console);

  void action_comment (Component::Action::Step step);
  void action_goto (const std::string& target);
  void action_look (const std::string& target);
  void action_move (Component::Action::Step step);
  void action_pick_animation (Component::Action::Step step);
  void action_set_state (Component::Action::Step step);
  void action_show (Component::Action::Step step);

  void create_dialog (const std::string& text, std::vector<Component::Image_handle>& dialog);
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
