#ifndef SOSAGE_SYSTEM_ANIMATION_H
#define SOSAGE_SYSTEM_ANIMATION_H

#include <Sosage/Content.h>
#include <Sosage/third_party_config.h>

namespace Sosage::System
{

class Animation
{
private:

  Content& m_content;

public:

  Animation (Content& content);

  void main();
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_GRAPHIC_H
