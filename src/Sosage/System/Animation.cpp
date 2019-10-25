#include <Sosage/Component/Animation.h>
#include <Sosage/System/Animation.h>
#include <Sosage/Config.h>

namespace Sosage::System
{

Animation::Animation (Content& content)
  : m_content (content)
{

}

void Animation::main()
{
  std::vector<Component::Animation_handle> animations;

  for (const auto& e : m_content)
    if (Component::Animation_handle anim
        = Component::component_cast<Component::Animation>(e))
      animations.push_back(anim);

  for (const auto& animation : animations)
    animation->next_frame();
}

} // namespace Sosage::System
