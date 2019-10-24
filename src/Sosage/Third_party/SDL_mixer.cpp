#include <Sosage/Third_party/SDL_mixer.h>
#include <Sosage/Utils/error.h>

#ifdef SOSAGE_LINKED_WITH_SDL_MIXER

namespace Sosage::Third_party
{

SDL_mixer::SDL_mixer()
{
  if (Mix_OpenAudio (44100, MIX_DEFAULT_FORMAT, MIX_DEFAULT_CHANNELS, 1024) == -1)
    error ("Cannot initialized SDL Mixer");
}

SDL_mixer::~SDL_mixer()
{
  Mix_CloseAudio ();
}

} // namespace Sosage::Third_party

#endif
