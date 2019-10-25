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
  
  int frame = 0;
  while (this->running())
  {
    if (frame ++ == config().animation_frame_rate)
    {
      frame = 0;

      m_content.lock();
      for (const auto& e : m_content)
        if (Component::Animation_handle anim
            = Component::component_cast<Component::Animation>(e))
          animations.push_back(anim);
      m_content.unlock();

      for (const auto& animation : animations)
      {
        animation->lock();
        animation->next_frame();
        animation->unlock();
      }
      
    }
    this->wait();
  }
}

} // namespace Sosage::System
