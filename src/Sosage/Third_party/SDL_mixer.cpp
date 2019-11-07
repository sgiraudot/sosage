#include <Sosage/Third_party/SDL_mixer.h>
#include <Sosage/Utils/error.h>

namespace Sosage::Third_party
{

SDL_mixer::SDL_mixer()
{
  int init = Mix_OpenAudio (44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024);
  check (init != -1, "Cannot initialized SDL Mixer");
}

SDL_mixer::~SDL_mixer()
{
  Mix_CloseAudio ();
}

} // namespace Sosage::Third_party

