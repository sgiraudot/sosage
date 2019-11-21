#ifndef SOSAGE_SYSTEM_LOGIC_H
#define SOSAGE_SYSTEM_LOGIC_H

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

  Content& m_content;
  Component::Image_handle m_collision;
  Component::Text_handle m_chosen_verb;

public:

  Logic (Content& content);

  void main();
  bool exit();
  bool paused();

private:
  void compute_path_from_target (Component::Position_handle target);
  void detect_collision (Component::Position_handle mouse);
  void update_interface ();
  void update_debug_info (Component::Debug_handle debug_info);
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
