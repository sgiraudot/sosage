#include <Sosage/Component/Animation.h>

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

      m_content.lock_components();
      for (const auto& e : m_content)
      {
        Component::Animation_handle animation;
      
        for (const auto& c : e.second)
          if (c.first == "image")
            animation = Component::component_cast<Component::Animation>(c.second);
      
        if (animation)
          animations.push_back (animation);
      }
      m_content.unlock_components();

      for (const auto& animation : animations)
      {
        animations->lock();
        animations->next_frame();
        animations->unlock();
      }
      
    }
    this->wait();
  }
}

} // namespace Sosage::System
