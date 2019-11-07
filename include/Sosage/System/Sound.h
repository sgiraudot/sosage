#ifndef SOSAGE_SYSTEM_SOUND_H
#define SOSAGE_SYSTEM_SOUND_H

#include <Sosage/Content.h>
#include <Sosage/Core/Sound.h>

namespace Sosage::System
{

class Sound
{
  Content& m_content;
  Core::Sound m_core;

public:

  Sound (Content& content);

  void main();
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_SOUND_H
