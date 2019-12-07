#ifndef SOSAGE_SYSTEM_ANIMATION_H
#define SOSAGE_SYSTEM_ANIMATION_H

#include <Sosage/Component/Animation.h>
#include <Sosage/Component/Path.h>
#include <Sosage/Content.h>

namespace Sosage::System
{

class Animation
{
private:

  Content& m_content;

public:

  Animation (Content& content);

  void main();
  
  void generate_random_idle_animation (Component::Animation_handle image,
                                       Component::Animation_handle head,
                                       const Vector& direction);

  void generate_random_idle_head_animation (Component::Animation_handle head,
                                            const Vector& direction);
  void generate_random_idle_body_animation (Component::Animation_handle body,
                                            const Vector& direction);

private:
  
  void compute_movement_from_path (Component::Path_handle path);
  void set_move_animation (Component::Animation_handle image,
                           Component::Animation_handle head,
                           const Vector& direction);
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_GRAPHIC_H
