#ifndef SOSAGE_CORE_SOUND_H
#define SOSAGE_CORE_SOUND_H

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

#include <Sosage/Third_party/SDL_mixer.h>
namespace Sosage::Core
{
typedef Third_party::SDL_mixer Sound;
}

#else

namespace Sosage::Core
{

class Sound
{
public:

  Sound() { }
  ~Sound() { }
};
  
}

#endif

#endif

