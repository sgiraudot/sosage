#ifndef SOSAGE_SYSTEM_INTERFACE_H
#define SOSAGE_SYSTEM_INTERFACE_H

#include <Sosage/Component/Image.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Content.h>

#include <vector>

namespace Sosage::System
{

class Interface
{
  Content& m_content;
  Component::Image_handle m_collision;
  
  const int m_action_min_height;
  const int m_interface_min_height;

  int m_action_height;
  double m_verb_scale;

  std::vector<Component::Image_handle> m_verbs;

public:

  Interface (Content& content);

  void main();

  void init();

private:
  void update_responsive();
  void detect_collision (Component::Position_handle mouse);
  void update_action ();
  void update_inventory ();

};


} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_INTERFACE_H
