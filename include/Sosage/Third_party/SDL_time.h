#ifndef SOSAGE_THIRD_PARTY_SDL_TIME_H
#define SOSAGE_THIRD_PARTY_SDL_TIME_H

#include <Sosage/Utils/error.h>
#include <Sosage/Config.h>

#include <SDL2/SDL_timer.h>

namespace Sosage::Third_party
{

class SDL_time
{
public:

  typedef Uint32 Unit;
  typedef Uint32 Duration;

  static Unit now() { return SDL_GetTicks(); }

  static void wait (const Duration& d) { SDL_Delay (d); }
};

} // namespace Sosage::Third_party

#endif // SOSAGE_THIRD_PARTY_SDL_TIME_H
