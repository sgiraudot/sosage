#ifndef SOSAGE_SYSTEM_LOGIC_H
#define SOSAGE_SYSTEM_LOGIC_H

#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Component/Position.h>
#include <Sosage/Content.h>

namespace Sosage::System
{

class Logic
{
private:

  Content& m_content;

public:

  Logic (Content& content);

  void main();
  bool exit();

private:

  void compute_path_from_target (Component::Position_handle target);
  void compute_movement_from_path (Component::Path_handle path);
  void set_move_animation (Component::Animation_handle image,
                           Component::Animation_handle head,
                           const Vector& direction);
public:
  void generate_random_idle_animation (Component::Animation_handle image,
                                       Component::Animation_handle head,
                                       const Vector& direction);
                           
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_LOGIC_H
