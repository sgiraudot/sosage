#ifndef SOSAGE_SYSTEM_SOUND_H
#define SOSAGE_SYSTEM_SOUND_H

#include <Sosage/Content.h>
#include <Sosage/Utils/thread.h>
#include <Sosage/third_party_config.h>

namespace Sosage::System
{

class Sound : public Threadable
{
  Content& m_content;
  Sound_core m_core;

public:

  Sound (Content& content);

  void main();
};

} // namespace Sosage::System

#endif // SOSAGE_SYSTEM_SOUND_H
