#ifndef SOSAGE_SYSTEM_LOGIC_H
#define SOSAGE_SYSTEM_LOGIC_H

#include <Sosage/Component/Action.h>
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

  typedef std::pair<std::size_t, Component::Handle> Timed_handle;
  std::vector<Timed_handle> m_timed;

  Content& m_content;
  std::size_t m_current_time;

  Component::Action_handle m_current_action;
  std::size_t m_next_step;

public:

  Logic (Content& content);

  void main(const std::size_t& current_time);
  bool exit();
  bool paused();

private:
  void compute_path_from_target (Component::Position_handle target);
  void update_debug_info (Component::Debug_handle debug_info);

  void action_comment (Component::Action::Step step);
  void action_goto (const std::string& target);
  void action_look (const std::string& target);
  void action_move (Component::Action::Step step);
  void action_pick_animation (Component::Action::Step step);
  void action_set_state (Component::Action::Step step);

  void create_dialog (const std::string& text, std::vector<Component::Image_handle>& dialog);
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
