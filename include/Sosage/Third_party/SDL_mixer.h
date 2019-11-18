#ifndef SOSAGE_THIRD_PARTY_SDL_MIXER_H
#define SOSAGE_THIRD_PARTY_SDL_MIXER_H

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

#include <SDL2/SDL_mixer.h>

namespace Sosage::Third_party
{

class SDL_mixer
{
private:

public:

  SDL_mixer ();
  ~SDL_mixer ();
};

} // namespace Sosage::Third_party

#endif

#endif // SOSAGE_THIRD_PARTY_SDL_H
